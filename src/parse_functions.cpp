/* **************************************************************************
 * Core parsing functions
 *
 * Deals with parsing previously tokenized data into meaningful game objects
 * and properties.
 *
 * Part of GTRPE by Gren Drake
 * **************************************************************************/
#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "parsestate.h"
#include "builderror.h"
#include "gamedata.h"
#include "build.h"
#include "origin.h"
#include "token.h"

#include "bytestream.h"


struct OpcodeDef {
    enum CodeOpcode {
        Return      = 0,
        Push0       = 1,
        Push1       = 2,
        PushNeg1    = 3,
        Push8       = 4,
        Push16      = 5,
        Push32      = 6,
        Store       = 7,
        Say         = 10,
        SayUnsigned = 11,
        SayChar     = 12,
        StackPop    = 13, // remove the top item from the stack
        StackDup    = 14, // duplicate the top item on the stack
        StackPeek   = 15, // peek at the stack item X items from the top
        StackSize   = 16, // get the current size of the stack
        Call        = 17, // call a value as a function
        CallMethod  = 18, // call an object property as a function
        Self        = 19, // get object the current function is a property of
        GetProp     = 20,
        HasProp     = 21, // check if property is set on object
        SetProp     = 22, // set object property to value
        GetItem     = 23, // get item from list (index) or map (key)
        HasItem     = 24, // check if index (for list) or key (for map) exists
        GetSize     = 25, // get size of list or map
        SetItem     = 26, // set item in list (by index) of map (by key)
        TypeOf      = 27, // get value type
        CompareTypes        = 30, // compare the types of two values and push the result
        Compare             = 31, // compare two values and push the result
        Jump                = 32, // unconditional jump
        JumpZero            = 33, // jump if top of stack == 0
        JumpNotZero         = 34, // jump if top of stack != 0
        JumpLessThan        = 35, // jump if top of stack < 0
        JumpLessThanEqual   = 36, // jump if top of stack <= 0
        JumpGreaterThan     = 37, // jump if top of stack > 0
        JumpGreaterThanEqual= 38, // jump if top of stack >= 0
        Add         = 40,
        Sub         = 41,
        Mult        = 42,
        Div         = 43,
    };

    std::string name;
    int code;
    int arg_size;
};


OpcodeDef opcodes[] = {
    {   "return",       OpcodeDef::Return       },
    {   "push0",        OpcodeDef::Push0        },
    {   "push1",        OpcodeDef::Push1        },
    {   "push-1",       OpcodeDef::PushNeg1     },
    {   "push8",        OpcodeDef::Push8        },
    {   "push16",       OpcodeDef::Push16       },
    {   "push32",       OpcodeDef::Push32       },
    {   "store",        OpcodeDef::Store        },
    {   "say",          OpcodeDef::Say          },
    {   "say-unsigned", OpcodeDef::SayUnsigned  },
    {   "say-char",     OpcodeDef::SayChar      },
    {   "pop",          OpcodeDef::StackPop     },
    {   "stack-dup",    OpcodeDef::StackDup     },
    {   "stack-peek",   OpcodeDef::StackPeek    },
    {   "stack-size",   OpcodeDef::StackSize    },
    {   "call",         OpcodeDef::Call         },
    {   "call-method",  OpcodeDef::CallMethod   },
    {   "self",         OpcodeDef::Self         },
    {   "get-prop",     OpcodeDef::GetProp      },
    {   "has-prop",     OpcodeDef::HasProp      },
    {   "set-prop",     OpcodeDef::SetProp      },
    {   "get-item",     OpcodeDef::GetItem      },
    {   "has-item",     OpcodeDef::HasItem      },
    {   "get-size",     OpcodeDef::GetSize      },
    {   "set-item",     OpcodeDef::SetItem      },
    {   "type-of",      OpcodeDef::TypeOf       },
    {   "cmp-type",     OpcodeDef::CompareTypes         },
    {   "cmp",          OpcodeDef::Compare              },
    {   "jmp",          OpcodeDef::Jump                 },
    {   "jeq",          OpcodeDef::JumpZero             },
    {   "jz",           OpcodeDef::JumpZero             },
    {   "jneq",         OpcodeDef::JumpNotZero          },
    {   "jnz",          OpcodeDef::JumpNotZero          },
    {   "jlt",          OpcodeDef::JumpLessThan         },
    {   "jlte",         OpcodeDef::JumpLessThanEqual    },
    {   "jgt",          OpcodeDef::JumpGreaterThan      },
    {   "jgte",         OpcodeDef::JumpGreaterThanEqual },
    {   "add",          OpcodeDef::Add          },
    {   "sub",          OpcodeDef::Sub          },
    {   "mult",         OpcodeDef::Mult         },
    {   "div",          OpcodeDef::Div          },
    {   ""                                      }
};

static void parse_asm_function(GameData &gamedata, FunctionDef *function, ParseState &state);
static void bytecode_push_value(ByteStream &bytecode, Value::Type type, int32_t value);

const OpcodeDef* getOpcode(const std::string &name) {
    for (const OpcodeDef &code : opcodes) {
        if (code.name == name) return &code;
    }
    return nullptr;
}

void bytecode_push_value(ByteStream &bytecode, Value::Type type, int32_t value) {
    if (value == 0) {
        bytecode.add_8(OpcodeDef::Push0);
        bytecode.add_8(type);
    } else if (value == 1) {
        bytecode.add_8(OpcodeDef::Push1);
        bytecode.add_8(type);
    } else if (value == -1) {
        bytecode.add_8(OpcodeDef::PushNeg1);
        bytecode.add_8(type);
    } else if (value >= INT8_MIN && value <= INT8_MAX) {
        bytecode.add_8(OpcodeDef::Push8);
        bytecode.add_8(type);
        bytecode.add_8(value);
    } else if (value >= INT16_MIN && value <= INT16_MAX) {
        bytecode.add_8(OpcodeDef::Push16);
        bytecode.add_8(type);
        bytecode.add_16(value);
    } else {
        bytecode.add_8(OpcodeDef::Push32);
        bytecode.add_8(type);
        bytecode.add_32(value);
    }
}

struct Backpatch {
    unsigned position;
    std::string name;
};
void parse_asm_function(GameData &gamedata, FunctionDef *function, ParseState &state) {
    std::map<std::string, unsigned> labels;
    std::vector<Backpatch> patches;

    while (!state.at_end()) {
        int ident;
        const OpcodeDef *opcode;
        const SymbolDef *symbol;
        switch(state.here()->type) {
            case Token::String:
                ident = gamedata.getStringId(state.here()->text);
                bytecode_push_value(function->code, Value::String, ident);
                break;
            case Token::Identifier: {
                // is opcode name
                opcode = getOpcode(state.here()->text);
                if (opcode) {
                    function->code.add_8(opcode->code);
                    break;
                }
                // is global symbol
                symbol = gamedata.symbols.get(state.here()->text);
                if (symbol) {
                    bytecode_push_value(function->code, symbol->value.type, symbol->value.value);
                    break;
                }
                // is local name
                auto localIter = std::find(function->local_names.begin(), function->local_names.end(), state.here()->text);
                if (localIter != function->local_names.end()) {
                    int localNumber = std::distance(function->local_names.begin(), localIter);
                    bytecode_push_value(function->code, Value::LocalVar, localNumber);
                    break;
                }
                // is label
                if (state.peek() && state.peek()->type == Token::Colon) {
                    auto labelIter = labels.find(state.here()->text);
                    if (labelIter != labels.end()) {
                        std::stringstream ss;
                        ss << "Label " << state.here()->text << " already defined.";
                        throw BuildError(state.here()->origin, ss.str());
                    }
                    labels.insert(std::make_pair(state.here()->text, function->code.size()));
                    state.next();
                    break;
                }
                // presume its a label
                auto labelIter = labels.find(state.here()->text);
                if (labelIter != labels.end()) {
                    bytecode_push_value(function->code, Value::JumpTarget, labelIter->second);
                } else {
                    function->code.add_8(OpcodeDef::Push32);
                    function->code.add_8(Value::JumpTarget);
                    unsigned labelPos = function->code.size();
                    patches.push_back(Backpatch{labelPos, state.here()->text});
                    function->code.add_32(0xFFFFFFFF);
                }
                break;
            }
            case Token::Integer:
                bytecode_push_value(function->code, Value::Integer, state.here()->value);
                break;
            case Token::Property:
                bytecode_push_value(function->code, Value::Property, state.here()->value);
                break;
            default:
                throw BuildError(state.here()->origin, "Unexpected token in function.");
        }
        state.next();
    }

    for (const Backpatch &patch : patches) {
        auto labelIter = labels.find(patch.name);
        if (labelIter != labels.end()) {
            function->code.overwrite_32(patch.position, labelIter->second);
        } else {
            std::stringstream ss;
            ss << "Unknown symbol " << patch.name << " in function " << function->name << '.';
            throw BuildError(ss.str());
        }
    }
}

int parse_functions(GameData &gamedata) {
    for (FunctionDef *function : gamedata.functions) {
        if (function == nullptr) continue;
        ParseState state = {
            gamedata,
            function->tokens,
            function->tokens.cbegin()
        };

        function->codePosition = gamedata.bytecode.size();
        if (state.at_end()) {
            // empty function
        } else if (state.matches("asm")) {
            state.next();
            parse_asm_function(gamedata, function, state);
        } else {
            throw BuildError(function->origin, "Unknown function type.");
        }
        function->code.add_8(OpcodeDef::Return);
        function->code.padTo(4);
        gamedata.bytecode.append(function->code);
    }

    return 1;
}