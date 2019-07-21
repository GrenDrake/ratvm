#ifndef GAMEDATA_H
#define GAMEDATA_H

#include <string>
#include <map>
#include <vector>
#include "bytestream.h"
#include "gameerror.h"
#include "stack.h"
#include "value.h"

const int FILETYPE_ID = 0x47505254;
const int HEADER_SIZE = 64;

struct DataItem {
    unsigned ident;
    int srcFile, srcLine, srcName;
    bool gcMark;
};

struct StringDef : public DataItem {
    std::string text;
};

struct ListDef : public DataItem {
    std::vector<Value> items;
};
struct MapDef : public DataItem  {
    struct Row {
        Value key, value;
    };
    std::vector<Row> rows;
};
struct ObjectDef : public DataItem  {
    std::map<unsigned, Value> properties;

    Value get(unsigned propId) const;
    bool has(unsigned propId) const;
    void set(unsigned propId, const Value &value);
};
struct FunctionDef : public DataItem  {
    int arg_count;
    int local_count;
    unsigned position;
};

enum class OptionType {
    None, Choice, Key, Line
};

struct GameOption {
    int strId;
    Value value;
    Value extra;
    int hotkey;
};

struct GameData {
    GameData() : gameLoaded(false), mCallCount(0) { }
    void load(const std::string &filename);
    void dump() const;

    StringDef& getString(int index) {
        if (index < 0 || index >= static_cast<int>(strings.size())) {
            throw GameError("Tried to access invalid string number "
                            + std::to_string(index));
        }
        return strings[index];
    }
    ListDef& getList(int index) {
        if (index < 0 || index >= static_cast<int>(lists.size())) {
            throw GameError("Tried to access invalid list number "
                            + std::to_string(index));
        }
        return lists[index];
    }
    MapDef& getMap(int index) {
        if (index < 0 || index >= static_cast<int>(maps.size())) {
            throw GameError("Tried to access invalid map number "
                            + std::to_string(index));
        }
        return maps[index];
    }
    ObjectDef& getObject(int index) {
        if (index < 0 || index >= static_cast<int>(objects.size())) {
            throw GameError("Tried to access invalid object number "
                            + std::to_string(index));
        }
        return objects[index];
    }
    FunctionDef& getFunction(int index) {
        if (index < 0 || index >= static_cast<int>(functions.size())) {
            throw GameError("Tried to access invalid function number "
                            + std::to_string(index));
        }
        return functions[index];
    }

    std::string getSource(const Value &value);
    Value runFunction(unsigned functionId, std::vector<Value> argList);
    void say(const std::string &what);
    void say(const Value &what);

    OptionType optionType;
    std::vector<GameOption> options;
    int optionFunction;
    std::string textBuffer;

    bool gameLoaded;
    int mainFunction;
    std::vector<StringDef> strings;
    std::vector<ListDef> lists;
    std::vector<MapDef> maps;
    std::vector<ObjectDef> objects;
    std::vector<FunctionDef> functions;
    ByteStream bytecode;
    unsigned staticStrings;
    unsigned staticLists;
    unsigned staticMaps;
    unsigned staticObjects;
    Value noneValue;

    gtCallStack callStack;
private:
    Value runFunctionCore(unsigned functionId, std::vector<Value> argList);

    unsigned mCallCount;
};

void gameloop(GameData &gamedata, bool doQuick);

#endif
