#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "gamedata.h"
#include "bytestream.h"
#include "value.h"

const unsigned char string_xor_key = 0x7B;

uint32_t read_32(std::istream &in);
uint16_t read_16(std::istream &in);
uint8_t read_8(std::istream &in);
std::string read_str(std::istream &in);
Value read_value(std::istream &in);


void GameData::load(const std::string &filename) {
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

    // skip header
    inf.seekg(HEADER_SIZE);

    // READ STRINGS
    staticStrings = read_32(inf);
    for (unsigned i = 0; i < staticStrings; ++i) {
        StringDef def;
        def.ident = i;
        def.text = read_str(inf);
        strings.push_back(def);
    }

    // // READ LISTS
    lists.push_back(ListDef{});
    staticLists = read_32(inf); // dummy list so index matches IDs
    for (unsigned i = 0; i < staticLists; ++i) {
        ListDef def;
        def.ident = i + 1;
        def.srcName = -1;
        def.srcFile = read_32(inf);
        def.srcLine = read_32(inf);
        unsigned itemCount = read_16(inf);
        for (unsigned j = 0; j < itemCount; ++j) {
            Value value;
            value.type = static_cast<Value::Type>(read_8(inf));
            value.value = read_32(inf);
            def.items.push_back(value);
        }
        lists.push_back(def);
    }

    // READ MAPS
    maps.push_back(MapDef{});
    staticMaps = read_32(inf); // dummy map so index matches IDs
    for (unsigned i = 0; i < staticMaps; ++i) {
        MapDef def;
        def.ident = i + 1;
        def.srcName = -1;
        def.srcFile = read_32(inf);
        def.srcLine = read_32(inf);
        unsigned itemCount = read_16(inf);
        for (unsigned j = 0; j < itemCount; ++j) {
            Value v1, v2;
            v1.type = static_cast<Value::Type>(read_8(inf));
            v1.value = read_32(inf);
            v2.type = static_cast<Value::Type>(read_8(inf));
            v2.value = read_32(inf);
            def.rows.push_back(MapDef::Row{v1,v2});
        }
        maps.push_back(def);
    }

    // READ OBJECTS
    objects.push_back(ObjectDef{});
    staticObjects = read_32(inf);
    for (unsigned i = 0; i < staticObjects; ++i) {
        ObjectDef def;
        def.ident = i + 1;
        def.srcName = read_32(inf);
        def.srcFile = read_32(inf);
        def.srcLine = read_32(inf);
        unsigned itemCount = read_16(inf);
        for (unsigned j = 0; j < itemCount; ++j) {
            unsigned propId = read_16(inf);
            Value value;
            value.type = static_cast<Value::Type>(read_8(inf));
            value.value = read_32(inf);
            def.properties.insert(std::make_pair(propId, value));
        }
        objects.push_back(def);
    }

    // READ FUNCTION HEADERS
    functions.push_back(FunctionDef{});
    unsigned functionCount = read_32(inf);
    for (unsigned i = 0; i < functionCount; ++i) {
        FunctionDef def;
        def.ident = i + 1;
        def.srcName = read_32(inf);
        def.srcFile = read_32(inf);
        def.srcLine = read_32(inf);
        def.arg_count = read_16(inf);
        def.local_count = read_16(inf);
        def.position = read_32(inf);
        functions.push_back(def);
    }

    // READ FUNCTION BYTECODE
    unsigned bytecodeSize = read_32(inf);
    for (unsigned i = 0; i < bytecodeSize; ++i) {
        bytecode.add_8(read_8(inf));
    }

    // VERIFY END OF FILE
    inf.get();
    if (!inf.eof()) {
        std::cerr << "End of file not reached at end of game data.\n";
        return;
    }

    noneValue = Value();
    gameLoaded = true;
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
        text[i] = read_8(in) ^ string_xor_key;
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
