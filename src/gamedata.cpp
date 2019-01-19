/* **************************************************************************
 * Definitions for the GameData class
 *
 * Part of GTRPE by Gren Drake
 * **************************************************************************/
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>

#include "gamedata.h"
#include "symboltable.h"
#include "builderror.h"
#include "origin.h"
#include "token.h"

GameData::GameData()
: nextAnonymousId(firstAnonymousId) {
    lists.push_back(nullptr);
    maps.push_back(nullptr);
    functions.push_back(nullptr);
}

GameData::~GameData() {
    for (GameObject *object : objects) {
        delete object;
    }
    for (GameList *list : lists) {
        delete list;
    }
    for (GameMap *map : maps) {
        delete map;
    }
    for (FunctionDef *function : functions) {
        delete function;
    }
}

unsigned GameData::getPropertyId(const std::string &name) {
    auto existing = std::find(propertyNames.begin(), propertyNames.end(), name);
    if (existing != propertyNames.end()) {
        return existing - propertyNames.begin();
    }
    propertyNames.push_back(name);
    return propertyNames.size() - 1;
}

const std::string* GameData::getPropertyName(unsigned id) const {
    if (id >= propertyNames.size()) {
        return nullptr;
    }
    return &propertyNames[id];
}

unsigned GameData::getStringId(const std::string &name) {
    auto existing = std::find(stringTable.begin(), stringTable.end(), name);
    if (existing != stringTable.end()) {
        return existing - stringTable.begin();
    }
    stringTable.push_back(name);
    return stringTable.size() - 1;
}

const std::string& GameData::getString(unsigned id) const {
    static std::string badIdString("(invalid string id)");
    if (id >= stringTable.size()) {
        return badIdString;
    }
    return stringTable[id];
}


bool propertySorter(const GameProperty &left, const GameProperty &right) {
    return left.id < right.id;
}

void GameData::organize() {
    for (GameObject *object : objects) {
        std::sort(object->properties.begin(), object->properties.end(), propertySorter);
    }
}

FunctionDef* GameData::functionByName(const std::string &name) {
    for (FunctionDef *func : functions) {
        if (func && func->name == name) {
            return func;
        }
    }
    return nullptr;
}
