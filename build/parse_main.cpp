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
static void parse_default(GameData &gamedata, ParseState &state);
static int parse_flags(GameData &gamedata, ParseState &state);
static int parse_function(GameData &gamedata, ParseState &state, const std::string &defaultName);
static int parse_list(GameData &gamedata, ParseState &state);
static int parse_map(GameData &gamedata, ParseState &state);
static int parse_object(GameData &gamedata, ParseState &state, const std::string &defaultName);
static Value parse_value(GameData &gamedata, ParseState &state, const std::string &defaultName);

///////////////////////////////////////////////////////////////////////////////
// Parse a value (formerly constant) declaration
void parse_constant(GameData &gamedata, ParseState &state) {
    Origin origin = state.here()->origin;
    state.next(); // skip "constant"
    try {
        state.require(Token::Identifier);
    } catch (BuildError &e) {
        gamedata.errors.push_back(Error{e.getOrigin(), e.getMessage()});
        state.next();
        return;
    }
    const std::string &constantName = state.here()->text;
    state.next();
    Value value = parse_value(gamedata, state, constantName);
    if (value.type == Value::Object || value.type == Value::Function) {
        std::stringstream ss;
        ss << "Declaration of " << constantName << " cannot declare objects or functions.";
        gamedata.errors.push_back(Error{state.here()->origin, ss.str()});
    }
    gamedata.symbols.add(SymbolDef(origin,
                                    constantName,
                                    value));
    // state.skip(Token::Semicolon);
}

///////////////////////////////////////////////////////////////////////////////
// Parse a default declaration value
static void parse_default(GameData &gamedata, ParseState &state) {
    Origin origin = state.here()->origin;
    state.next(); // skip "default"
    try {
        state.require(Token::Identifier);
    } catch (BuildError &e) {
        gamedata.errors.push_back(Error{e.getOrigin(), e.getMessage()});
        state.next();
        return;
    }
    const std::string &defaultName = state.here()->text;
    state.next();
    Value value = parse_value(gamedata, state, defaultName);
    if (value.type == Value::Object || value.type == Value::Function) {
        std::stringstream ss;
        ss << "Declaration of default value for " << defaultName;
        ss << " cannot declare objects or functions.";
        gamedata.errors.push_back(Error{state.here()->origin, ss.str()});
    }
    const SymbolDef *oldDefault = gamedata.defaults.get(defaultName);
    if (!oldDefault) {
        gamedata.defaults.add(SymbolDef(origin,
                                        defaultName,
                                        value));
    } else {
        std::stringstream ss;
        ss << "Default value for " << defaultName;
        ss << " already declared at " << oldDefault->origin << ".";
        gamedata.errors.push_back(Error{origin, ss.str()});
    }
}

///////////////////////////////////////////////////////////////////////////////
// Parse a set of flags
int parse_flags(GameData &gamedata, ParseState &state) {
    static int nextFlagsId = 0;
    const Origin &origin = state.here()->origin;
    state.skip("flags");
    state.skip(Token::OpenParan);

    FlagSet flagset;
    flagset.origin = origin;
    flagset.globalId = nextFlagsId++;
    // values
    while (!state.eof() && !state.matches(Token::CloseParan)) {
        if (state.here()->type == Token::Integer) {
            flagset.values.push_back(Value{Value::Integer, state.here()->value});
        } else if (state.here()->type == Token::Identifier) {
            flagset.values.push_back(Value{Value::Symbol, 0, state.here()->text});
        } else {
            std::stringstream ss;
            ss << "Invalid token " << state.here()->type << " in flags.";
            gamedata.errors.push_back(Error{state.here()->origin, ss.str()});
        }
        state.next();
    }
    gamedata.flagsets.push_back(flagset);
    state.next();
    return flagset.globalId;
}

///////////////////////////////////////////////////////////////////////////////
// Parse a function
int parse_function(GameData &gamedata, ParseState &state, const std::string &defaultName) {
    static int nextFunctionId = 1;
    bool isAsm = false;
    const Origin &origin = state.here()->origin;
    if (state.matches("asm_function")) {
        isAsm = true;
        state.skip("asm_function");
    } else {
        state.skip("function");
    }

    std::string funcName;
    if (state.matches(Token::Identifier)) {
        funcName = state.here()->text;
        gamedata.symbols.add(SymbolDef(origin,
                                        funcName,
                                        Value{Value::Function, nextFunctionId}));
        state.next();
    } else {
        funcName = defaultName;
    }
    state.skip(Token::OpenParan);

    FunctionDef *function = new FunctionDef;
    function->origin = origin;
    function->origin.fileNameString = gamedata.getStringId(origin.file);
    function->name = funcName;
    function->nameString = gamedata.getStringId(funcName);
    function->globalId = nextFunctionId++;
    function->isAsm = isAsm;
    gamedata.functions.push_back(function);
    // hidden "self" argument
    ++function->argument_count;
    function->local_names.push_back("self");
    // arguments / locals
    bool doingLocals = false;
    while (!state.eof() && !state.matches(Token::CloseParan)) {
        if (state.matches(Token::Colon)) {
            doingLocals = true;
            state.next();
            continue;
        }
        try {
            state.require(Token::Identifier);
        } catch (BuildError &e) {
            gamedata.errors.push_back(Error{e.getOrigin(), e.getMessage()});
            state.next();
            continue;
        }
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
    list->origin.fileNameString = gamedata.getStringId(origin.file);
    list->globalId = nextListId++;
    while (!state.matches(Token::CloseSquare)) {
        if (state.eof()) {
            delete list;
            gamedata.errors.push_back(Error{origin, "Unexpected end of file in list."});
            return 0;
        }
        Value value = parse_value(gamedata, state, "");
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
    map->origin.fileNameString = gamedata.getStringId(origin.file);
    map->globalId = nextMapId++;
    state.skip(Token::OpenBrace);
    while (!state.matches(Token::CloseBrace)) {
        if (state.eof()) {
            delete map;
            gamedata.errors.push_back(Error{origin, "Unexpected end of file in map."});
            return 0;
        }
        Value v1 = parse_value(gamedata, state, "");
        Value v2{Value::None};
        try {
            state.skip(Token::Colon);
            v2 = parse_value(gamedata, state, "");
        } catch (BuildError &e) {
            gamedata.errors.push_back(Error{e.getOrigin(), e.getMessage()});
        }

        map->rows.push_back(GameMap::MapRow{v1, v2});
    }

    state.next();
    return map->globalId;
}

///////////////////////////////////////////////////////////////////////////////
// Parse a single object
int parse_object(GameData &gamedata, ParseState &state, const std::string &defaultName) {
    static int nextObjectId = 1;
    unsigned internalNameId = gamedata.getPropertyId("internal_name");

    const Origin &origin = state.here()->origin;
    state.next(); // skip "object"

    std::string objectName = "";
    if (state.matches(Token::Identifier)) {
        objectName = state.here()->text;
        state.next();
    } else {
        objectName = defaultName;
    }

    GameObject *object = new GameObject;
    object->origin = origin;
    object->origin.fileNameString = gamedata.getStringId(origin.file);
    object->name = objectName;
    object->nameString = gamedata.getStringId(objectName);
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
        try {
            state.require(Token::Property);
            propId = state.here()->value;
            propName = state.here()->text;
        } catch (BuildError &e) {
            gamedata.errors.push_back(Error{e.getOrigin(), e.getMessage()});
            propId = -1;
            propName = "bad_prop";
        }
        const Origin &propOrigin = state.here()->origin;
        state.next();

        if (state.eof()) {
            gamedata.errors.push_back(Error{origin, "Unexpected end of file in object definition."});
            return 0;
        }
        std::string defaultName = objectName + "." + propName;
        Value value = parse_value(gamedata, state, defaultName);
        if (propId != -1) {
            try {
                object->addProperty(propOrigin, propId, value);
            } catch (BuildError &e) {
                gamedata.errors.push_back(Error{e.getOrigin(), e.getMessage()});
            }
        }
    }

    state.next();
    return object->globalId;
}

///////////////////////////////////////////////////////////////////////////////
// Parse a data value
Value parse_value(GameData &gamedata, ParseState &state, const std::string &defaultName) {
    const Origin &origin = state.here()->origin;
    Value value;

    if (state.eof()) {
        gamedata.errors.push_back(Error{origin, "Unexpected end of file."});
        return Value{};
    } else if (state.matches("object")) {
        int newId = parse_object(gamedata, state, defaultName);
        value = Value{Value::Object, newId};
    } else if (state.matches("flags")) {
        int newId = parse_flags(gamedata, state);
        value = Value{Value::FlagSet, newId};
    } else if (state.matches("function") || state.matches("asm_function")) {
        int newId = parse_function(gamedata, state, defaultName);
        value = Value{Value::Function, newId};
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
        try {
            state.require(Token::Identifier);
        } catch (BuildError &e) {
            gamedata.errors.push_back(Error{e.getOrigin(), e.getMessage()});
            state.next();
            continue;
        }

        if (state.matches("declare")) {
            parse_constant(gamedata, state);
        } else if (state.matches("default")) {
            parse_default(gamedata, state);
        } else if (state.matches("object")) {
            int objectId = parse_object(gamedata, state, "");
            if (objectId > 0) {
                GameObject *object = gamedata.objects[objectId];
                if (object && object->name.empty()) {
                    gamedata.errors.push_back(Error{object->origin, "Anonymous object at top level"});
                }
            }
        } else if (state.matches("function") || state.matches("asm_function")) {
            int functionId = parse_function(gamedata, state, "");
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