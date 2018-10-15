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
    int ident;
};

struct GameList {
    Origin origin;
    int ident;
    std::vector<Value> items;
};

struct GameMap {
    struct MapRow {
        Value key;
        Value value;
    };

    Origin origin;
    int ident;
    std::vector<MapRow> rows;
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
};

class GameData {
public:
    GameData();
    ~GameData();

    int getAnomyousId();
    unsigned getPropertyId(const std::string &name);
    const std::string* getPropertyName(unsigned id) const;
    unsigned getStringId(const std::string &name);
    const std::string& getString(unsigned id) const;
    GameObject* objectById(int ident);
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

    int nextAnonymousId;
};

std::ostream& operator<<(std::ostream &out, const Value &property);

void dump_gamedata(GameData &gamedata, std::ostream &out);
void dump_token_list(const std::vector<Token> &tokens, std::ostream &out);

#endif

