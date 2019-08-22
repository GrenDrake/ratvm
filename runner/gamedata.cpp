#include <iostream>
#include <sstream>
#include <string>
#include "gamedata.h"

Value ListDef::get(int key) const {
    if (key < 0 || key >= static_cast<int>(items.size())) {
        return Value(Value::Integer, 0);
    } else {
        return items[key];
    }
}

bool ListDef::has(int key) const {
    if (key < 0 || key >= static_cast<int>(items.size())) {
        return false;
    } else {
        return true;
    }
}

void ListDef::set(int key, const Value &value) {
    if (key >= 0 && key < static_cast<int>(items.size())) {
        items[key] = value;
    }
}

void ListDef::del(int key) {
    if (key >= 0 || key < static_cast<int>(items.size())) {
        items.erase(items.begin() + key);
    }
}


Value MapDef::get(const Value &key) const {
    for (const Row &row : rows) {
        if (row.key == key) return row.value;
    }
    return Value(Value::Integer, 0);
}

bool MapDef::has(const Value &key) const {
    for (const Row &row : rows) {
        if (row.key == key) return true;
    }
    return false;
}

void MapDef::set(const Value &key, const Value &value) {
    for (Row &row : rows) {
        if (row.key == key) {
            row.value = value;
            return;
        }
    }
    rows.push_back(Row{key, value});
}

void MapDef::del(const Value &key) {
    auto iter = rows.begin();
    while (iter != rows.end()) {
        if (iter->key == key) {
            rows.erase(iter);
            return;
        }
        ++iter;
    }
}


Value ObjectDef::get(GameData &gamedata, unsigned propId, bool checkParent) const {
    auto iter = properties.find(propId);
    if (iter == properties.end()) {
        if (checkParent) {
            Value parent = get(gamedata, PROP_PARENT, false);
            if (parent.type == Value::Object) {
                return gamedata.getObject(parent.value).get(gamedata, propId);
            }
        }
        return Value{Value::Integer, 0};
    }
    Value result = iter->second;
    result.selfObj = ident;
    return result;
}

bool ObjectDef::has(unsigned propId) const {
    auto iter = properties.find(propId);
    if (iter == properties.end()) return false;
    return true;
}

void ObjectDef::set(unsigned propId, const Value &value) {
    properties[propId] = value;
}


std::string GameData::getSource(const Value &value) {
    std::string text;
    const DataItem *item = nullptr;

    switch(value.type) {
        case Value::None:
        case Value::Integer:
        case Value::Property:
        case Value::String:
        case Value::LocalVar:
        case Value::VarRef:
        case Value::JumpTarget:
        case Value::TypeId:
            return "";
        case Value::Map:
            item = &getMap(value.value);
            break;
        case Value::List:
            item = &getList(value.value);
            break;
        case Value::Object:
            item = &getObject(value.value);
            break;
        case Value::Function:
            item = &getFunction(value.value);
            break;
    }

    if (item->srcFile == -1) return "no debug info";
    if (item->srcFile == -2) return "dynamic";

    if (item->srcName >= 0) text = "\"" + getString(item->srcName).text + "\" ";
    if (item->srcLine >= 0) text += getString(item->srcFile).text
                                  + ":" + std::to_string(item->srcLine);
    else                    text += getString(item->srcFile).text;

    return text;
}

void GameData::setExtra(const Value &newValue) {
    if (extraValue >= 0) {
        callStack.getStack().setArg(extraValue, newValue);
    }
}

void GameData::say(const std::string &what) {
    textBuffer += what;
}

void GameData::say(const Value &what) {
    say(asString(what));
}

Value GameData::makeNew(Value::Type type) {
    switch(type) {
        case Value::List: {
            ListDef newDef;
            newDef.ident = lists.size();
            newDef.srcFile = newDef.srcLine = newDef.srcName = -ORIGIN_DYNAMIC;
            lists.push_back(newDef);
            return Value(Value::List, newDef.ident);
        }
        case Value::Map: {
            MapDef newDef;
            newDef.ident = maps.size();
            newDef.srcFile = newDef.srcLine = newDef.srcName = -ORIGIN_DYNAMIC;
            maps.push_back(newDef);
            return Value(Value::Map, newDef.ident);
        }
        case Value::Object: {
            ObjectDef newDef;
            newDef.ident = objects.size();
            newDef.srcFile = newDef.srcLine = newDef.srcName = -ORIGIN_DYNAMIC;
            objects.push_back(newDef);
            return Value(Value::Object, newDef.ident);
        }
        case Value::String: {
            StringDef newDef;
            newDef.ident = strings.size();
            newDef.srcFile = newDef.srcLine = newDef.srcName = -ORIGIN_DYNAMIC;
            strings.push_back(newDef);
            return Value(Value::String, newDef.ident);
        }
        default:
            std::stringstream ss;
            ss << "Tried to create new value of type " << type;
            ss << " which cannot be instantiated.";
            throw GameError(ss.str());
    }
}

bool GameData::isStatic(const Value &what) const {
    switch(what.type) {
        case Value::Object: return what.value < staticObjects;
        case Value::List:   return what.value < staticLists;
        case Value::Map:    return what.value < staticMaps;
        case Value::String: return what.value < staticStrings;
        default:            return true;
    }
}

void GameData::stringAppend(const Value &stringId, const Value &toAppend, bool upperFirst) {
    stringId.requireType(Value::String);
    std::string newText = asString(toAppend);
    StringDef &strDef = getString(stringId.value);
    if (upperFirst && !newText.empty()) {
        // FIXME: not UTF-8 aware
        newText[0] = std::toupper(newText[0]);
    }
    strDef.text += newText;
}

std::string GameData::asString(const Value &value) {
    switch(value.type) {
        case Value::String: {
            const StringDef &strDef = getString(value.value);
            return strDef.text;
        }
        case Value::Integer: {
            return std::to_string(value.value);
        }
        default: {
            std::stringstream ss;
            ss << value;
            return ss.str();
        }
    }
}
