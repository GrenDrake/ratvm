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
#include "opcode.h"
#include "expression.h"

#include "bytestream.h"

void parse_std_function(GameData &gamedata, FunctionDef *function, ParseState &state);
static int bytecode_push_value(ByteStream &bytecode, Value::Type type, int32_t value);
void build_function(FunctionDef *function);

ListValue parse_listvalue(GameData &gamedata, FunctionDef *function, ParseState &state);
List* parse_list(GameData &gamedata, FunctionDef *function, ParseState &state);


int bytecode_push_value(ByteStream &bytecode, Value::Type type, int32_t value) {
    if (type == Value::None) {
        bytecode.add_8(OpcodeDef::PushNone);
        return 1;
    } else if (value == 0) {
        bytecode.add_8(OpcodeDef::Push0);
        bytecode.add_8(type);
        return 2;
    } else if (value == 1) {
        bytecode.add_8(OpcodeDef::Push1);
        bytecode.add_8(type);
        return 2;
    } else if (value >= INT8_MIN && value <= INT8_MAX) {
        bytecode.add_8(OpcodeDef::Push8);
        bytecode.add_8(type);
        bytecode.add_8(value);
        return 3;
    } else if (value >= INT16_MIN && value <= INT16_MAX) {
        bytecode.add_8(OpcodeDef::Push16);
        bytecode.add_8(type);
        bytecode.add_16(value);
        return 4;
    } else {
        bytecode.add_8(OpcodeDef::Push32);
        bytecode.add_8(type);
        bytecode.add_32(value);
        return 6;
    }
}

bool nameInUse(GameData &gamedata, FunctionDef *function, const std::string &name, unsigned localId) {
    if (function->isAsm && getOpcode(name)) return true;
    if (gamedata.symbols.get(name)) return true;
    if (getReservedWord(name).handler != nullptr) {
        return true;
    }
    for (const auto &labelIter : function->labels) {
        if (labelIter.first == name) {
            return true;
        }
    }
    for (std::vector<LocalDef>::size_type i = 0; i < function->locals.size(); ++i) {
        if (i == localId) continue;
        if (name == function->locals[i].name) {
            return true;
        }
    }
    return false;
}

Value evalIdentifier(GameData &gamedata, FunctionDef *function, const std::string identifier) {
    // is opcode name
    const OpcodeDef *opcode = getOpcode(identifier);
    if (opcode) {
        Value result = Value{Value::Opcode, 0, identifier};
        result.opcode = opcode;
        return result;
    }

    // is reserved word
    if (getReservedWord(identifier).handler != nullptr) {
        return Value{Value::Reserved, 0, identifier};
    }

    // is global symbol
    const SymbolDef *symbol = gamedata.symbols.get(identifier, true);
    if (symbol) {
        return symbol->value;
    }

    // is local name
    int localNumber = function->getLocalNumber(identifier);
    if (localNumber >= 0) {
        return Value{Value::LocalVar, localNumber};
    }
    return Value{Value::Symbol, 0, identifier};
}

ListValue parse_listvalue(GameData &gamedata, FunctionDef *function, ParseState &state) {
    const Token *here = state.here();
    ListValue newValue = {Origin(), Value{Value::None}};
    if (!here) return newValue;

    switch(here->type) {
        case Token::Integer:
            newValue = ListValue{here->origin,  Value{Value::Integer, here->value} };
            break;
        case Token::Property:
            newValue = ListValue{here->origin,  Value{Value::Property, here->value} };
            break;
        case Token::String: {
            int ident = gamedata.getStringId(state.here()->text);
            newValue = ListValue{here->origin,  Value{Value::String, ident} };
            break; }
        case Token::OpenParan: {
            List *sublist = parse_list(gamedata, function, state);
            newValue = ListValue{here->origin,  Value{Value::Expression}, sublist };
            break; }
        case Token::Identifier: {
            Value result = evalIdentifier(gamedata, function, here->text);
            newValue = ListValue{here->origin,  result };
            break; }
        case Token::Indirection:
            newValue = ListValue{here->origin,  Value{Value::Indirection} };
            break;
        case Token::Colon:
            newValue = ListValue{here->origin,  Value{Value::Colon} };
            break;
        default: {
            std::stringstream ss;
            ss << "Unexpected type " << here->type << " found in list.";
            gamedata.addError(here->origin, ErrorMsg::Error, ss.str());
            here = state.next();
            return newValue; }
    }
    here = state.next();
    return newValue;
}

List* parse_list(GameData &gamedata, FunctionDef *function, ParseState &state) {
    try {
        state.require(Token::OpenParan);
    } catch (BuildError &e) {
        gamedata.addError(e.getOrigin(), ErrorMsg::Error, e.getMessage());
        return nullptr;
    }
    state.next();

    List *list = new List;
    const Token *here = state.here();
    while (here && here->type != Token::CloseParan) {
        ListValue nextValue = parse_listvalue(gamedata, function, state);
        list->values.push_back(nextValue);
        here = state.here();
    }
    return list;
}

void FunctionBuilder::build(const AsmValue *value) {
    if (value->value.type == Value::Symbol) {
        auto labelIter = forFunction->labels.find(value->value.text);
        if (labelIter != forFunction->labels.end()) {
            bytecode_push_value(forFunction->code, Value::JumpTarget, labelIter->second);
        } else {
            forFunction->code.add_8(OpcodeDef::Push32);
            forFunction->code.add_8(Value::JumpTarget);
            patches.push_back(Backpatch{forFunction->code.size(), value->value.text, value->getOrigin()});
            forFunction->code.add_32(0xFFFFFFFF);
        }
    } else {
        bytecode_push_value(forFunction->code, value->value.type, value->value.value);
        if (value->value.type == Value::LocalVar || value->value.type == Value::VarRef) {
            LocalDef *def = forFunction->getLocal(value->value.value);
            if (def) ++def->reads;
        }
    }
}
void FunctionBuilder::build(const AsmLabel *label) {
    forFunction->labels.insert(std::make_pair(label->text, forFunction->code.size()));
}
void FunctionBuilder::build(const AsmOpcode *opcode) {
    forFunction->code.add_8(opcode->opcode);
}

void build_function(GameData &gamedata, FunctionDef *function) {
    FunctionBuilder builder{function, gamedata};

    function->addValue(function->origin, Value{Value::Integer, 0});
    function->addOpcode(function->origin, OpcodeDef::Return);
    for (const AsmLine *line : function->asmCode) {
        line->build(builder);
    }

    for (const Backpatch &patch : builder.patches) {
        auto labelIter = function->labels.find(patch.name);
        if (labelIter != function->labels.end()) {
            function->code.overwrite_32(patch.position, labelIter->second);
        } else {
            std::stringstream ss;
            ss << "Undefined symbol " << patch.name << '.';
            gamedata.addError(patch.origin, ErrorMsg::Error, ss.str());
        }
    }
}

void parse_std_function(GameData &gamedata, FunctionDef *function, ParseState &state) {
    std::vector<List*> lists;

    while (!state.at_end()) {
        List *list = parse_list(gamedata, function, state);
        if (!list) return;
        lists.push_back(list);
        state.next();
    }

    if (gamedata.errors.empty()) {
        for (List *l : lists) {
            process_list(gamedata, function, l);
            function->addOpcode(function->origin, OpcodeDef::StackPop);
        }
    }
    build_function(gamedata, function);

    for (List *l : lists) {
        delete l;
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

        for (std::vector<LocalDef>::size_type i = 0; i < function->locals.size(); ++i) {
            if (nameInUse(gamedata, function, function->locals[i].name, i)) {
                std::stringstream ss;
                ss << "Local name \"" << function->locals[i].name << "\" already in use.";
                gamedata.addError(function->origin, ErrorMsg::Error, ss.str());
            }
        }

        function->codePosition = gamedata.bytecode.size();
        parse_std_function(gamedata, function, state);
        function->code.padTo(4);
        gamedata.bytecode.append(function->code);
        function->codeEndPosition = gamedata.bytecode.size();

        for (const LocalDef &def : function->locals) {
            if (def.reads == 0) {
                gamedata.addError(function->origin, ErrorMsg::Warning, "Local variable " + def.name + " not used.");
            }
        }
    }

    return 1;
}