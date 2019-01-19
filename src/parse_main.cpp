/* **************************************************************************
 * Core parsing functions
 *
 * Deals with parsing previously tokenized data into meaningful game objects
 * and properties.
 *
 * Part of GTRPE by Gren Drake
 * **************************************************************************/

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "parsestate.h"
#include "builderror.h"
#include "gamedata.h"
#include "build.h"
#include "origin.h"
#include "token.h"


///////////////////////////////////////////////////////////////////////////////
// Parse function declarations

static void parse_constant(GameData &gamedata, ParseState &state);
static int parse_function(GameData &gamedata, ParseState &state);
static int parse_list(GameData &gamedata, ParseState &state);
static int parse_map(GameData &gamedata, ParseState &state);
static int parse_object(GameData &gamedata, ParseState &state);
static Value parse_value(GameData &gamedata, ParseState &state);

///////////////////////////////////////////////////////////////////////////////
// Parse a value (formerly constant) declaration
void parse_constant(GameData &gamedata, ParseState &state) {
    Origin origin = state.here()->origin;
    state.next(); // skip "constant"
    state.require(Token::Identifier);
    const std::string &constantName = state.here()->text;
    state.next();
    Value value = parse_value(gamedata, state);
    gamedata.symbols.add(SymbolDef(origin,
                                    constantName,
                                    value));
    state.skip(Token::Semicolon);
}

///////////////////////////////////////////////////////////////////////////////
// Parse a function
int parse_function(GameData &gamedata, ParseState &state) {
    static int nextFunctionId = 1;
    const Origin &origin = state.here()->origin;
    state.skip("function");

    std::string funcName;
    if (state.matches(Token::Identifier)) {
        funcName = state.here()->text;
        gamedata.symbols.add(SymbolDef(origin,
                                        funcName,
                                        Value{Value::Node, nextFunctionId}));
        state.next();
    }
    state.skip(Token::OpenParan);

    FunctionDef *function = new FunctionDef;
    function->origin = origin;
    function->name = funcName;
    function->globalId = nextFunctionId++;
    gamedata.functions.push_back(function);
    // arguments / locals
    bool doingLocals = false;
    while (!state.eof() && !state.matches(Token::CloseParan)) {
        if (state.matches(Token::Colon)) {
            doingLocals = true;
            state.next();
            continue;
        }
        state.require(Token::Identifier);
        if (doingLocals)    ++function->local_count;
        else                ++function->argument_count;
        function->local_names.push_back(state.here()->text);
        state.next();
    }
    state.next();
    state.skip(Token::OpenBrace);
    while (!state.matches(Token::CloseBrace)) {
        if (state.eof()) {
            gamedata.errors.push_back(Error{origin, "Unexpected end of file in function."});
            return 0;
        }
        function->tokens.push_back(*state.here());
        state.next();
    }
    state.next();
    return function->globalId;
}

///////////////////////////////////////////////////////////////////////////////
// Parse a list declaration
int parse_list(GameData &gamedata, ParseState &state) {
    static int nextListId = 1;
    const Origin &origin = state.here()->origin;
    state.skip(Token::OpenSquare);

    GameList *list = new GameList;
    gamedata.lists.push_back(list);
    list->origin = origin;
    list->globalId = nextListId++;
    while (!state.matches(Token::CloseSquare)) {
        if (state.eof()) {
            delete list;
            gamedata.errors.push_back(Error{origin, "Unexpected end of file in list."});
            return 0;
        }
        Value value = parse_value(gamedata, state);
        list->items.push_back(value);
    }

    state.next();
    return list->globalId;
}

///////////////////////////////////////////////////////////////////////////////
// Parse a map declaration
int parse_map(GameData &gamedata, ParseState &state) {
    static int nextMapId = 1;
    const Origin &origin = state.here()->origin;

    GameMap *map = new GameMap;
    gamedata.maps.push_back(map);
    map->origin = origin;
    map->globalId = nextMapId++;
    state.skip(Token::OpenBrace);
    while (!state.matches(Token::CloseBrace)) {
        if (state.eof()) {
            delete map;
            gamedata.errors.push_back(Error{origin, "Unexpected end of file in map."});
            return 0;
        }
        Value v1 = parse_value(gamedata, state);
        state.skip(Token::Colon);
        Value v2 = parse_value(gamedata, state);

        map->rows.push_back(GameMap::MapRow{v1, v2});
    }

    state.next();
    return map->globalId;
}

///////////////////////////////////////////////////////////////////////////////
// Parse a single object
int parse_object(GameData &gamedata, ParseState &state) {
    static int nextObjectId = 1;
    unsigned internalNameId = gamedata.getPropertyId("internal-name");

    const Origin &origin = state.here()->origin;
    state.next(); // skip "object"

    std::string objectName = "";
    if (state.matches(Token::Identifier)) {
        objectName = state.here()->text;
        state.next();
    }

    GameObject *object = new GameObject;
    object->origin = origin;
    object->name = objectName;
    object->globalId = nextObjectId++;
    gamedata.objects.push_back(object);
    if (!objectName.empty()) {
        gamedata.symbols.add(SymbolDef(origin,
                                        objectName,
                                        Value{Value::Object, object->globalId}));
        int nameStringId = gamedata.getStringId(objectName);
        object->addProperty(state.here()->origin, internalNameId,
                            Value{Value::String, nameStringId});
    }

    while (!state.matches(Token::Semicolon)) {
        unsigned propId = 0;
        std::string propName;
        if (state.matches(Token::Identifier)) {
            propName = state.here()->text;
            propId = gamedata.getPropertyId(propName);
        } else {
            state.require(Token::Property);
            propId = state.here()->value;
            propName = state.here()->text;
        }
        const Origin &propOrigin = state.here()->origin;
        state.next();

        if (state.eof()) {
            gamedata.errors.push_back(Error{origin, "Unexpected end of file in object definition."});
            return 0;
        }
        Value value = parse_value(gamedata, state);
        if (value.type == Value::Node) {
            FunctionDef *function = gamedata.functions[value.value];
            if (function->name.empty()) {
                function->name = objectName + "." + propName;
            }
        }
        object->addProperty(propOrigin, propId, value);
    }

    state.next();
    return object->globalId;
}

///////////////////////////////////////////////////////////////////////////////
// Parse a data value
Value parse_value(GameData &gamedata, ParseState &state) {
    const Origin &origin = state.here()->origin;
    Value value;

    if (state.eof()) {
        gamedata.errors.push_back(Error{origin, "Unexpected end of file."});
        return Value{};
    } else if (state.matches("object")) {
        int newId = parse_object(gamedata, state);
        value = Value{Value::Object, newId};
    } else if (state.matches("function")) {
        int newId = parse_function(gamedata, state);
        value = Value{Value::Node, newId};
    } else if (state.matches(Token::Integer)) {
        int newId = state.here()->value;
        value = Value{Value::Integer, newId};
        state.next();
    } else if (state.matches(Token::Property)) {
        int newId = state.here()->value;
        value = Value{Value::Property, newId};
        state.next();
    } else if (state.matches(Token::String)) {
        int newId = gamedata.getStringId(state.here()->text);
        value = Value{Value::String, newId};
        state.next();
    } else if (state.matches(Token::Identifier)) {
        value = Value{Value::Symbol, 0, state.here()->text};
        state.next();
    } else if (state.matches(Token::OpenSquare)) {
        int newId = parse_list(gamedata, state);
        value = Value{Value::List, newId};
    } else if (state.matches(Token::OpenBrace)) {
        int newId = parse_map(gamedata, state);
        value = Value{Value::Map, newId};
    } else {
        std::stringstream ss;
        ss << "Encountered value of invalid type " << state.here()->type << ".";
        gamedata.errors.push_back(Error{origin, ss.str()});
    }
    return value;
}

///////////////////////////////////////////////////////////////////////////////
// Parse a list of tokens
int parse_tokens(GameData &gamedata, const std::vector<Token> &tokens) {
    ParseState state = {
        gamedata,
        tokens,
        tokens.cbegin()
    };

    while (!state.at_end()) {
        if (state.matches(Token::EndOfFile)) {
            state.next();
            continue;
        }
        state.require(Token::Identifier);

        if (state.matches("value")) {
            parse_constant(gamedata, state);
        } else if (state.matches("object")) {
            int objectId = parse_object(gamedata, state);
            if (objectId > 0) {
                GameObject *object = gamedata.objects[objectId - 1];
                if (object && object->name.empty()) {
                    gamedata.errors.push_back(Error{object->origin, "Anonymous object at top level"});
                }
            }
        } else if (state.matches("function")) {
            int functionId = parse_function(gamedata, state);
            if (functionId > 0) {
                FunctionDef *function = gamedata.functions[functionId];
                if (function && function->name.empty()) {
                    gamedata.errors.push_back(Error{function->origin, "Anonymous function at top level"});
                }
            }
        } else {
            const Token *token = state.here();
            std::stringstream ss;
            ss << "Unexpected top level directive " << token->text << ".";
            gamedata.errors.push_back(Error{token->origin, ss.str()});
            state.next();
        }

    }

    return 1;
}