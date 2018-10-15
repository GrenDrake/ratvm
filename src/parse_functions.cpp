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
        Return       = 0,
        Push0        = 1,
        Push1        = 2,
        PushNeg1     = 3,
        Push8        = 4,
        Push16       = 5,
        Push32       = 6,
        Say          = 10,
        GetProp      = 20,
    };

    std::string name;
    int code;
    int arg_size;
};


OpcodeDef opcodes[] = {
    {   "return",       OpcodeDef::Return   },
    {   "push0",        OpcodeDef::Push0    },
    {   "push1",        OpcodeDef::Push1    },
    {   "push-1",       OpcodeDef::PushNeg1 },
    {   "push8",        OpcodeDef::Push8    },
    {   "push16",       OpcodeDef::Push16   },
    {   "push32",       OpcodeDef::Push32   },
    {   "say",          OpcodeDef::Say      },
    {   "get-prop",     OpcodeDef::GetProp  },
    {   ""                          }
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

void parse_asm_function(GameData &gamedata, FunctionDef *function, ParseState &state) {
    std::map<std::string, unsigned> labels;

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
                opcode = getOpcode(state.here()->text);
                if (opcode) {
                    function->code.add_8(opcode->code);
                    break;
                }
                symbol = gamedata.symbols.get(state.here()->text);
                if (symbol) {
                    bytecode_push_value(function->code, symbol->value.type, symbol->value.value);
                    break;
                }
                auto localIter = std::find(function->local_names.begin(), function->local_names.end(), state.here()->text);
                if (localIter != function->local_names.end()) {
                    int localNumber = std::distance(function->local_names.begin(), localIter);
                    bytecode_push_value(function->code, Value::LocalVar, localNumber);
                    break;
                }
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
                std::stringstream ss;
                ss << "Unknown symbol " << state.here()->text << '.';
                throw BuildError(state.here()->origin, ss.str());
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

    for (auto iter : labels) {
        std::cout << iter.first << ": " << iter.second << '\n';
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