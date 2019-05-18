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

static void parse_asm_function(GameData &gamedata, FunctionDef *function, ParseState &state);
void parse_std_function(GameData &gamedata, FunctionDef *function, ParseState &state);
static void bytecode_push_value(ByteStream &bytecode, Value::Type type, int32_t value);

std::vector<std::string> reservedWords{
    "asm",
    "do_while",
    "get",
    "if",
    "proc",
    "say",
    "set",
    "while",
};

void bytecode_push_value(ByteStream &bytecode, Value::Type type, int32_t value) {
    if (type == Value::None) {
        bytecode.add_8(OpcodeDef::PushNone);
    } else if (value == 0) {
        bytecode.add_8(OpcodeDef::Push0);
        bytecode.add_8(type);
    } else if (value == 1) {
        bytecode.add_8(OpcodeDef::Push1);
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

bool nameInUse(GameData &gamedata, FunctionDef *function, const std::string &name, unsigned localId) {
    if (function->isAsm && getOpcode(name)) return true;
    if (gamedata.symbols.get(name)) return true;
    if (std::find(reservedWords.begin(), reservedWords.end(), name) != reservedWords.end()) {
        return true;
    }
    for (const auto &labelIter : function->labels) {
        if (labelIter.first == name) {
            return true;
        }
    }
    for (unsigned i = 0; i < function->local_names.size(); ++i) {
        if (i == localId) continue;
        if (name == function->local_names[i]) {
            return true;
        }
    }
    return false;
}

struct Backpatch {
    unsigned position;
    std::string name;
    Origin origin;
};
void parse_asm_function(GameData &gamedata, FunctionDef *function, ParseState &state) {
    const OpcodeDef *opcode;
    const SymbolDef *symbol;
    std::vector<Backpatch> patches;

    while (!state.at_end()) {
        int ident;
        switch(state.here()->type) {
            case Token::String:
                ident = gamedata.getStringId(state.here()->text);
                bytecode_push_value(function->code, Value::String, ident);
                break;
            case Token::Indirection: {
                // variable name reference
                state.next();
                state.require(Token::Type::Identifier);
                int argumentNumber = -1;
                for (unsigned i = 0; i < function->local_names.size(); ++i) {
                    if (function->local_names[i] == state.here()->text) {
                        argumentNumber = i;
                    }
                }
                if (argumentNumber < 0) {
                    std::stringstream ss;
                    ss << "Symbol \"" << state.here()->text << "\" is not a local variable name.";
                    gamedata.errors.push_back(Error{state.here()->origin, ss.str()});
                }
                bytecode_push_value(function->code, Value::VarRef, argumentNumber);
                break; }
            case Token::Identifier: {
                // is label
                if (state.peek() && state.peek()->type == Token::Colon) {
                    if (nameInUse(gamedata, function, state.here()->text, -1)) {
                        std::stringstream ss;
                        ss << "Symbol \"" << state.here()->text << "\" already defined.";
                        gamedata.errors.push_back(Error{state.here()->origin, ss.str()});
                    }
                    function->labels.insert(std::make_pair(state.here()->text, function->code.size()));
                    state.next();
                    break;
                }
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
                // presume its a label
                auto labelIter = function->labels.find(state.here()->text);
                if (labelIter != function->labels.end()) {
                    bytecode_push_value(function->code, Value::JumpTarget, labelIter->second);
                } else {
                    function->code.add_8(OpcodeDef::Push32);
                    function->code.add_8(Value::JumpTarget);
                    unsigned labelPos = function->code.size();
                    patches.push_back(Backpatch{labelPos, state.here()->text,state.here()->origin});
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
            default: {
                std::stringstream ss;
                ss << "Unexpected token " << state.here()->type << " in asm function body.";
                gamedata.errors.push_back(Error{state.here()->origin, ss.str()});
                }
        }
        state.next();
    }

    for (const Backpatch &patch : patches) {
        auto labelIter = function->labels.find(patch.name);
        if (labelIter != function->labels.end()) {
            function->code.overwrite_32(patch.position, labelIter->second);
        } else {
            std::stringstream ss;
            ss << "Unknown symbol " << patch.name << " in function " << function->name << '.';
            gamedata.errors.push_back(Error{patch.origin, ss.str()});
        }
    }
}




List* parse_list(GameData &gamedata, FunctionDef *function, ParseState &state) {
    try {
        state.require(Token::OpenParan);
    } catch (BuildError &e) {
        gamedata.errors.push_back(Error{e.getOrigin(), e.getMessage()});
        return nullptr;
    }
    state.next();

    List *list = new List;
    const Token *here = state.here();
    while (here && here->type != Token::CloseParan) {
        switch(here->type) {
            case Token::Integer:
                list->values.push_back(ListValue{ Value{Value::Integer, here->value} });
                break;
            case Token::Property:
                list->values.push_back(ListValue{ Value{Value::Property, here->value} });
                break;
            case Token::String: {
                int ident = gamedata.getStringId(state.here()->text);
                list->values.push_back(ListValue{ Value{Value::String, ident} });
                break; }
            case Token::OpenParan: {
                List *sublist = parse_list(gamedata, function, state);
                list->values.push_back(ListValue{ Value{Value::Expression}, sublist });
                break; }
            case Token::Identifier: {
                // is opcode name
                if (getOpcode(state.here()->text)) {
                    list->values.push_back(ListValue{ Value{Value::Opcode, 0, here->text} });
                    break;
                }
                // is reserved word
                if (std::find(reservedWords.begin(), reservedWords.end(), here->text) != reservedWords.end()) {
                    list->values.push_back(ListValue{ Value{Value::Reserved, 0, here->text} });
                    break;
                }
                // is global symbol
                const SymbolDef *symbol = gamedata.symbols.get(state.here()->text);
                if (symbol) {
                    list->values.push_back(ListValue{ symbol->value });
                    break;
                }
                // is local name
                auto localIter = std::find(function->local_names.begin(), function->local_names.end(), state.here()->text);
                if (localIter != function->local_names.end()) {
                    int localNumber = std::distance(function->local_names.begin(), localIter);
                    list->values.push_back(ListValue{ Value{Value::LocalVar, localNumber} });
                    break;
                }

                std::stringstream ss;
                ss << "Undefined symbol " << here->text << '.';
                gamedata.errors.push_back(Error{here->origin, ss.str()});
                break; }
            default: {
                std::stringstream ss;
                ss << "Unexpected type " << here->type << '.';
                gamedata.errors.push_back(Error{here->origin, ss.str()});
                break; }
        }
        here = state.next();
    }
    return list;
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
            dump_list(l, std::cout);
            std::cout << "\n";
        }
    }

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

        for (unsigned i = 0; i < function->local_names.size(); ++i) {
            const std::string &name = function->local_names[i];
            if (nameInUse(gamedata, function, name, i)) {
                std::stringstream ss;
                ss << "Local name \"" << name << "\" already in use.";
                gamedata.errors.push_back(Error{function->origin, ss.str()});
            }
        }

        function->codePosition = gamedata.bytecode.size();
        if (function->isAsm) {
            parse_asm_function(gamedata, function, state);
        } else {
            parse_std_function(gamedata, function, state);
            // if (state.at_end()) {
            //     // empty function
            // } else {
            //     gamedata.errors.push_back(Error{function->origin, "Non-asm functions not implemented."});
            // }
        }
        function->code.add_8(OpcodeDef::Return);
        function->code.padTo(4);
        gamedata.bytecode.append(function->code);
        function->codeEndPosition = gamedata.bytecode.size();
    }

    return 1;
}