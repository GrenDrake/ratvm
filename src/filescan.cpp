#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "bytestream.h"
#include "value.h"

const int FILETYPE_ID = 0x47505254;

struct StringDef {
    int ident;
    std::string text;
};
struct ListDef {
    int ident;
    std::vector<Value> items;
};
struct MapDef {
    struct Row {
        Value key, value;
    };
    int ident;
    std::vector<Row> rows;
};
struct ObjectDef {
    int ident;
    std::map<unsigned, Value> properties;
};
struct FunctionDef {
    int ident;
    int arg_count;
    int local_count;
    unsigned position;
};

struct GameData {
    GameData() : gameLoaded(false) { }
    void load(const std::string filename);
    void dump() const;

    bool gameLoaded;
    int mainFunction;
    std::map<int, StringDef> strings;
    std::map<int, ListDef> lists;
    std::map<int, MapDef> maps;
    std::map<int, ObjectDef> objects;
    std::map<int, FunctionDef> functions;
    ByteStream bytecode;
};


uint32_t read_32(std::istream &in);
uint16_t read_16(std::istream &in);
uint8_t read_8(std::istream &in);
std::string read_str(std::istream &in);
Value read_value(std::istream &in);


void GameData::load(const std::string filename) {
    std::ifstream inf(filename);
    if (!inf) {
        std::cerr << "Could not open ~" << filename << "~.\n";
        return;
    }
    if(read_32(inf) != FILETYPE_ID) {
        std::cerr << '~' << filename << "~ is not a valid gamefile.\n";
        return;
    }
    int version = read_32(inf);
    if(version != 0) {
        std::cerr << '~' << filename << "~ has format version " << version;
        std::cerr << ", but only version 0 is supported.\n";
        return;
    }
    mainFunction = read_32(inf);
    unsigned count = 0;

    // READ STRINGS
    count = read_32(inf);
    for (unsigned i = 0; i < count; ++i) {
        StringDef def;
        def.ident = i;
        def.text = read_str(inf);
        strings.insert(std::make_pair(def.ident, def));
    }

    // READ LISTS
    count = read_32(inf);
    for (unsigned i = 0; i < count; ++i) {
        ListDef def;
        def.ident = read_32(inf);
        unsigned itemCount = read_16(inf);
        for (unsigned j = 0; j < itemCount; ++j) {
            Value value;
            value.type = static_cast<Value::Type>(read_8(inf));
            value.value = read_32(inf);
            def.items.push_back(value);
        }
        lists.insert(std::make_pair(def.ident, def));
    }

    count = read_32(inf);
    for (unsigned i = 0; i < count; ++i) {
        MapDef def;
        def.ident = read_32(inf);
        unsigned itemCount = read_16(inf);
        for (unsigned j = 0; j < itemCount; ++j) {
            Value v1, v2;
            v1.type = static_cast<Value::Type>(read_8(inf));
            v1.value = read_32(inf);
            v2.type = static_cast<Value::Type>(read_8(inf));
            v2.value = read_32(inf);
            def.rows.push_back(MapDef::Row{v1,v2});
        }
        maps.insert(std::make_pair(def.ident, def));
    }

    count = read_32(inf);
    for (unsigned i = 0; i < count; ++i) {
        ObjectDef def;
        def.ident = read_32(inf);
        unsigned itemCount = read_16(inf);
        for (unsigned j = 0; j < itemCount; ++j) {
            unsigned propId = read_16(inf);
            Value value;
            value.type = static_cast<Value::Type>(read_8(inf));
            value.value = read_32(inf);
            def.properties.insert(std::make_pair(propId, value));
        }
        objects.insert(std::make_pair(def.ident, def));
    }

    count = read_32(inf);
    for (unsigned i = 0; i < count; ++i) {
        FunctionDef def;
        def.ident = read_32(inf);
        def.arg_count = read_16(inf);
        def.local_count = read_16(inf);
        def.position = read_32(inf);
        functions.insert(std::make_pair(def.ident, def));
    }

    count = read_32(inf);
    for (unsigned i = 0; i < count; ++i) {
        bytecode.add_8(read_8(inf));
    }
    gameLoaded = true;
}

void GameData::dump() const {

    std::cout << "\n## Strings\n";
    for (const auto &stringDef : strings) {
        std::cout << '[' << stringDef.second.ident << "] ~";
        std::cout << stringDef.second.text << "~\n";
    }

    std::cout << "\n## Lists\n";
    for (const auto &listDef : lists) {
        std::cout << '[' << listDef.second.ident << "] {";
        for (const Value &value : listDef.second.items) {
            std::cout << ' ' << value;
        }
        std::cout << " }\n";
    }

    std::cout << "\n## Maps\n";
    for (const auto &mapDef : maps) {
        std::cout << '[' << mapDef.second.ident << "] {";
        for (const MapDef::Row &row : mapDef.second.rows) {
            std::cout << " (" << row.key << ", " << row.value << ")";
        }
        std::cout << " }\n";
    }

    std::cout << "\n## Objects\n";
    for (const auto &objectDef : objects) {
        std::cout << '[' << objectDef.second.ident << "] {";
        for (const auto &property : objectDef.second.properties) {
            std::cout << " (" << property.first << ", " << property.second << ")";
        }
        std::cout << " }\n";
    }

    std::cout << "\n## Function Headers\n";
    for (const auto &functionDef : functions) {
        std::cout << '[' << functionDef.second.ident << "] args: ";
        std::cout << functionDef.second.arg_count << " locals: ";
        std::cout << functionDef.second.local_count << " position: ";
        std::cout << functionDef.second.position << "\n";
    }

    std::cout << "\n## Bytecode";
    bytecode.dump(std::cout, 0);
}


uint32_t read_32(std::istream &in) {
    uint32_t value;
    in.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

uint16_t read_16(std::istream &in) {
    uint16_t value;
    in.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

uint8_t read_8(std::istream &in) {
    uint8_t value;
    in.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

std::string read_str(std::istream &in) {
    int length = read_16(in);
    std::string text(length, ' ');
    for (int i = 0; i < length; ++i) {
        text[i] = read_8(in);
    }
    return text;
}

Value read_value(std::istream &in) {
    uint32_t valueSrc = read_32(in);
    Value value;
    value.type = static_cast<Value::Type>(valueSrc >> 28);
    value.value = valueSrc & 0x0FFFFFFF;
    return value;
}


int main(int argc, char *argv[]) {
    std::string gameFile = "game.bin";

    if (argc == 2)  gameFile = argv[1];
    if (argc >= 3 || gameFile == "-h" || gameFile == "--help") {
        std::cerr << "USAGE: ./filedump [game file]\n";
        return 1;
    }
    if (gameFile == "-v" || gameFile == "--version") {
        std::cerr << "FileDump utility for GTRPGE, V1.0\n";
        return 0;
    }

    GameData data;
    data.load(gameFile);
    if (!data.gameLoaded) return 1;
    data.dump();
    return 0;
}