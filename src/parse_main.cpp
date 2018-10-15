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
    const Origin &origin = state.here()->origin;
    state.skip("function");

    int functionId = 0;
    if (state.matches(Token::Integer)) {
        functionId = state.here()->value;
        if (functionId >= firstAnonymousId) {
            std::cerr << origin << " WARNING object IDs over ";
            std::cerr << firstAnonymousId;
            std::cerr << " are reserved for anonymous objects.\n";
        } else if (functionId == 0) {
            std::cerr << origin << " WARNING object ID 0 is reserved.\n";
        }
        state.next();
    } else {
        functionId = gamedata.getAnomyousId();
    }

    std::string funcName;
    if (state.matches(Token::Identifier)) {
        funcName = state.here()->text;
        gamedata.symbols.add(SymbolDef(origin,
                                        funcName,
                                        Value{Value::Node, functionId}));
        state.next();
    }
    state.skip(Token::OpenParan);

    FunctionDef *function = new FunctionDef;
    function->origin = origin;
    function->name = funcName;
    function->ident = functionId;
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
            throw BuildError(origin, "Unexpected end of file in function.");
        }
        function->tokens.push_back(*state.here());
        state.next();
    }
    state.next();
    return functionId;
}

///////////////////////////////////////////////////////////////////////////////
// Parse a list declaration
int parse_list(GameData &gamedata, ParseState &state) {
    int listId = gamedata.getAnomyousId();
    const Origin &origin = state.here()->origin;
    state.skip(Token::OpenSquare);

    GameList *list = new GameList;
    gamedata.lists.push_back(list);
    list->origin = origin;
    list->ident = listId;
    while (!state.matches(Token::CloseSquare)) {
        if (state.eof()) {
            delete list;
            throw BuildError(origin, "Unexpected end of file in list");
        }
        Value value = parse_value(gamedata, state);
        list->items.push_back(value);
    }

    state.next();
    return listId;
}

///////////////////////////////////////////////////////////////////////////////
// Parse a map declaration
int parse_map(GameData &gamedata, ParseState &state) {
    int listId = gamedata.getAnomyousId();
    const Origin &origin = state.here()->origin;

    GameMap *map = new GameMap;
    gamedata.maps.push_back(map);
    map->origin = origin;
    map->ident = listId;
    state.skip(Token::OpenBrace);
    while (!state.matches(Token::CloseBrace)) {
        if (state.eof()) {
            delete map;
            throw BuildError(origin, "Unexpected end of file in map");
        }
        Value v1 = parse_value(gamedata, state);
        state.skip(Token::Colon);
        Value v2 = parse_value(gamedata, state);

        map->rows.push_back(GameMap::MapRow{v1, v2});
    }

    state.next();
    return listId;
}

///////////////////////////////////////////////////////////////////////////////
// Parse a single object
int parse_object(GameData &gamedata, ParseState &state) {
    unsigned internalNameId = gamedata.getPropertyId("internal-name");
    unsigned objectIdId = gamedata.getPropertyId("ident");

    const Origin &origin = state.here()->origin;
    state.next(); // skip "object"

    state.require(Token::Integer);
    int objectId = state.here()->value;
    if (objectId == 0) {
        std::stringstream ss;
        ss << "Object id " << objectId << " not allowed to be zero.";
        throw BuildError(origin, ss.str());
    }
    GameObject *identOwner = gamedata.objectById(objectId);
    if (identOwner != nullptr) {
        std::stringstream ss;
        ss << "Object id " << objectId << " already in use by object ";
        ss << identOwner->name << '[' << identOwner->ident << "] @ ";
        ss << identOwner->origin << '.';
        throw BuildError(origin, ss.str());
    }
    state.next();

    std::string objectName = "";
    if (state.matches(Token::Identifier)) {
        objectName = state.here()->text;
        state.next();
    }

    GameObject *object = new GameObject;
    object->origin = origin;
    object->name = objectName;
    object->ident = objectId;
    gamedata.objects.push_back(object);
    if (!objectName.empty()) {
        gamedata.symbols.add(SymbolDef(origin,
                                        objectName,
                                        Value{Value::Object, objectId}));
        int nameStringId = gamedata.getStringId(objectName);
        object->addProperty(state.here()->origin, internalNameId,
                            Value{Value::String, nameStringId});
    }
    object->addProperty(state.here()->origin, objectIdId,
                        Value{Value::Integer, objectId});

    while (!state.matches(Token::Semicolon)) {
        state.require(Token::Property);
        const Origin &propOrigin = state.here()->origin;
        unsigned propId = state.here()->value;
        state.next();

        if (state.eof()) {
            throw BuildError(propOrigin, "Unexpected end of file in object property definition");
        }
        Value value = parse_value(gamedata, state);
        object->addProperty(propOrigin, propId, value);
    }

    state.next();
    return objectId;
}

///////////////////////////////////////////////////////////////////////////////
// Parse a data value
Value parse_value(GameData &gamedata, ParseState &state) {
    const Origin &origin = state.here()->origin;
    Value value;

    if (state.eof()) {
        throw BuildError(origin, "Unexpected end of file");
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
        throw BuildError(origin, ss.str());
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
            parse_object(gamedata, state);
        } else if (state.matches("function")) {
            parse_function(gamedata, state);
        } else {
            const Token *token = state.here();
            std::stringstream ss;
            ss << "Unexpected top level directive " << token->text << ".";
            throw BuildError(token->origin, ss.str());
        }

    }

    return 1;
}