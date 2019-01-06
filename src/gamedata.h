#ifndef GAMEDATA_H
#define GAMEDATA_H

#include "bytestream.h"
#include "origin.h"
#include "symboltable.h"
#include "token.h"
#include "value.h"
#include <string>
#include <vector>
#include <iosfwd>

const int firstAnonymousId = 10000000;

struct Error {
    Origin origin;
    std::string message;
};

struct GameProperty {
    Origin origin;
    unsigned id;
    Value value;
};

struct GameObject {
    void addProperty(const Origin &origin, unsigned id, const Value &value);
    const GameProperty* getProperty(unsigned id) const;

    Origin origin;
    std::vector<GameProperty> properties;
    std::string name;
    int globalId;
};

struct GameList {
    Origin origin;
    int ident;
    std::vector<Value> items;
    int globalId;
};

struct GameMap {
    struct MapRow {
        Value key;
        Value value;
    };

    Origin origin;
    int ident;
    std::vector<MapRow> rows;
    int globalId;
};

struct FunctionDef {
    FunctionDef()
    : argument_count(0), local_count(0)
    { }

    Origin origin;
    int ident;
    int argument_count;
    int local_count;
    std::vector<std::string> local_names;
    std::string name;
    std::vector<Token> tokens;
    unsigned codePosition;
    ByteStream code;
    int globalId;
};

class GameData {
public:
    GameData();
    ~GameData();

    unsigned getPropertyId(const std::string &name);
    const std::string* getPropertyName(unsigned id) const;
    unsigned getStringId(const std::string &name);
    const std::string& getString(unsigned id) const;
    void organize();
    FunctionDef* functionByName(const std::string &name);

    std::vector<Error> errors;
    SymbolTable symbols;
    std::vector<GameObject*> objects;
    std::vector<std::string> propertyNames;
    std::vector<std::string> stringTable;
    std::vector<GameList*> lists;
    std::vector<GameMap*> maps;
    std::vector<FunctionDef*> functions;
    ByteStream bytecode;

    unsigned stringsStart, listsStart, mapsStart, objectsStart;
    unsigned functionsStart, bytecodeStart, fileEnd;

    int nextAnonymousId;
};

std::ostream& operator<<(std::ostream &out, const Value &property);

void dump_gamedata(GameData &gamedata, std::ostream &out);
void dump_token_list(const std::vector<Token> &tokens, std::ostream &out);

#endif

