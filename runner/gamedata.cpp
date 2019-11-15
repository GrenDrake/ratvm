#include <algorithm>
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


GameData::~GameData() {
    for (ObjectDef *def : objects)  if (def) delete def;
    for (ListDef   *def : lists)    if (def) delete def;
    for (MapDef    *def : maps)     if (def) delete def;
    for (StringDef *def : strings)  if (def) delete def;
}

const StringDef& GameData::getString(int index) const {
    if (index < 0 || index >= static_cast<int>(strings.size()) || strings[index] == nullptr) {
        throw GameBadReference("Tried to access invalid string number "
                        + std::to_string(index));
    }
    return *strings[index];
}
StringDef& GameData::getString(int index) {
    if (index < 0 || index >= static_cast<int>(strings.size()) || strings[index] == nullptr) {
        throw GameBadReference("Tried to access invalid string number "
                        + std::to_string(index));
    }
    return *strings[index];
}
const ListDef& GameData::getList(int index) const {
    if (index <= 0 || index >= static_cast<int>(lists.size()) || lists[index] == nullptr) {
        throw GameBadReference("Tried to access invalid list number "
                        + std::to_string(index));
    }
    return *lists[index];
}
ListDef& GameData::getList(int index) {
    if (index <= 0 || index >= static_cast<int>(lists.size()) || lists[index] == nullptr) {
        throw GameBadReference("Tried to access invalid list number "
                        + std::to_string(index));
    }
    return *lists[index];
}
const MapDef& GameData::getMap(int index) const {
    if (index <= 0 || index >= static_cast<int>(maps.size()) || maps[index] == nullptr) {
        throw GameBadReference("Tried to access invalid map number "
                        + std::to_string(index));
    }
    return *maps[index];
}
MapDef& GameData::getMap(int index) {
    if (index <= 0 || index >= static_cast<int>(maps.size()) || maps[index] == nullptr) {
        throw GameBadReference("Tried to access invalid map number "
                        + std::to_string(index));
    }
    return *maps[index];
}
const ObjectDef& GameData::getObject(int index) const {
    if (index <= 0 || index >= static_cast<int>(objects.size()) || objects[index] == nullptr) {
        throw GameBadReference("Tried to access invalid object number "
                        + std::to_string(index));
    }
    return *objects[index];
}
ObjectDef& GameData::getObject(int index) {
    if (index <= 0 || index >= static_cast<int>(objects.size()) || objects[index] == nullptr) {
        throw GameBadReference("Tried to access invalid object number "
                        + std::to_string(index));
    }
    return *objects[index];
}
const FunctionDef& GameData::getFunction(int index) const {
    if (index <= 0 || index >= static_cast<int>(functions.size())) {
        throw GameBadReference("Tried to access invalid function number "
                        + std::to_string(index));
    }
    return functions[index];
}
FunctionDef& GameData::getFunction(int index) {
    if (index <= 0 || index >= static_cast<int>(functions.size())) {
        throw GameBadReference("Tried to access invalid function number "
                        + std::to_string(index));
    }
    return functions[index];
}


int GameData::collectGarbage() {
    // clear existing marks
    for (ObjectDef *obj  : objects) if (obj)  obj->gcMark = false;
    for (ListDef   *list : lists)   if (list) list->gcMark = false;
    for (MapDef    *map  : maps)    if (map)  map->gcMark = false;
    for (StringDef *str  : strings) if (str)  str->gcMark = false;

    // mark objects
    for (unsigned i = 0; i <= staticObjects; ++i) {
        ObjectDef *def = objects[i];
        if (def) mark(*def);
    }
    for (unsigned i = 0; i <= staticLists; ++i) {
        ListDef *def = lists[i];
        if (def) mark(*def);
    }
    for (unsigned i = 0; i <= staticMaps; ++i) {
        MapDef *def = maps[i];
        if (def) mark(*def);
    }
    for (unsigned i = 0; i <= staticStrings; ++i) {
        StringDef *def = strings[i];
        if (def) mark(*def);
    }
    // mark options
    for (GameOption &option : options) {
        mark(option.extra);
        mark(option.value);
        mark(Value(Value::String, option.strId));
    }
    for (int i = 0; i < callStack.size(); ++i) {
        const gtCallStack::Frame &frame = callStack[i];
        for (unsigned j = 0; j < frame.stack.size(); ++j) {
            mark(frame.stack[j]);
        }
        for (const Value &value : frame.stack.argList) {
            mark(value);
        }
    }

    // collect objects
    int collectionCount = 0;
    for (unsigned i = 0; i < objects.size(); ++i) {
        ObjectDef *def = objects[i];
        if (!def || def->gcMark) continue;
        delete def;
        objects[i] = nullptr;
        ++collectionCount;
    }
    for (unsigned i = 0; i < lists.size(); ++i) {
        ListDef *def = lists[i];
        if (!def || def->gcMark) continue;
        delete def;
        lists[i] = nullptr;
        ++collectionCount;
    }
    for (unsigned i = 0; i < maps.size(); ++i) {
        MapDef *def = maps[i];
        if (!def || def->gcMark) continue;
        delete def;
        maps[i] = nullptr;
        ++collectionCount;
    }
    for (unsigned i = 0; i < strings.size(); ++i) {
        StringDef *def = strings[i];
        if (!def || def->gcMark) continue;
        delete def;
        strings[i] = nullptr;
        ++collectionCount;
    }

    // trim containers
    while (!objects.empty() && objects.back() == nullptr) {
        objects.resize(objects.size() - 1);
    }
    while (!lists.empty() && lists.back() == nullptr) {
        lists.resize(lists.size() - 1);
    }
    while (!maps.empty() && maps.back() == nullptr) {
        maps.resize(maps.size() - 1);
    }
    while (!strings.empty() && strings.back() == nullptr) {
        strings.resize(strings.size() - 1);
    }

    return collectionCount;
}

void GameData::mark(ObjectDef &object) {
    object.gcMark = true;
    for (const auto &prop : object.properties) {
        mark(prop.second);
    }
}

void GameData::mark(ListDef &list) {
    list.gcMark = true;
    for (const Value &value : list.items) mark(value);
}

void GameData::mark(MapDef &map) {
    map.gcMark = true;
    for (const auto &row : map.rows) {
        mark(row.key);
        mark(row.value);
    }
}

void GameData::mark(StringDef &str) {
    str.gcMark = true;
}

void GameData::mark(const Value &value) {
    try {
        switch(value.type) {
            case Value::Object:
                mark(getList(value.value));
                break;
            case Value::List:
                mark(getList(value.value));
                break;
            case Value::Map:
                mark(getList(value.value));
                break;
            case Value::String:
                mark(getList(value.value));
                break;

            // remaining types not handled by garbage collector so just skip them
            case Value::None:
            case Value::Integer:
            case Value::Function:
            case Value::Property:
            case Value::TypeId:
            case Value::LocalVar:
            case Value::JumpTarget:
            case Value::VarRef:
                break;
        }
    } catch (const GameBadReference &e) {
        // do nothing; no need to mark non-existance objects
    }
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
            ListDef *newDef = new ListDef;
            newDef->ident = lists.size();
            newDef->srcFile = newDef->srcLine = newDef->srcName = -ORIGIN_DYNAMIC;
            lists.push_back(newDef);
            return Value(Value::List, newDef->ident);
        }
        case Value::Map: {
            MapDef *newDef = new MapDef;
            newDef->ident = maps.size();
            newDef->srcFile = newDef->srcLine = newDef->srcName = -ORIGIN_DYNAMIC;
            maps.push_back(newDef);
            return Value(Value::Map, newDef->ident);
        }
        case Value::Object: {
            ObjectDef *newDef = new ObjectDef;
            newDef->ident = objects.size();
            newDef->srcFile = newDef->srcLine = newDef->srcName = -ORIGIN_DYNAMIC;
            objects.push_back(newDef);
            return Value(Value::Object, newDef->ident);
        }
        case Value::String: {
            StringDef *newDef = new StringDef;
            newDef->ident = strings.size();
            newDef->srcFile = newDef->srcLine = newDef->srcName = -ORIGIN_DYNAMIC;
            strings.push_back(newDef);
            return Value(Value::String, newDef->ident);
        }
        default:
            std::stringstream ss;
            ss << "Tried to create new value of type " << type;
            ss << " which cannot be instantiated.";
            throw GameError(ss.str());
    }
}

Value GameData::makeNewString(const std::string &str) {
    Value newId = makeNew(Value::String);
    StringDef &def = getString(newId.value);
    def.text = str;
    return newId;
}

bool GameData::isStatic(const Value &what) const {
    switch(what.type) {
        case Value::Object: return static_cast<unsigned>(what.value) < staticObjects;
        case Value::List:   return static_cast<unsigned>(what.value) < staticLists;
        case Value::Map:    return static_cast<unsigned>(what.value) < staticMaps;
        case Value::String: return static_cast<unsigned>(what.value) < staticStrings;
        default:            return true;
    }
}

bool GameData::isValid(const Value &what) const {
    try {
        switch(what.type) {
            case Value::Object:     getObject(what.value);      break;
            case Value::List:       getList(what.value);        break;
            case Value::Map:        getMap(what.value);         break;
            case Value::String:     getString(what.value);      break;
            case Value::Function:   getFunction(what.value);    break;
            default:                return true;
        }
    } catch (const GameBadReference &error) {
        return false;
    }
    return true;
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

class ListItemSorter {
public:
    ListItemSorter(const GameData &gamedata)
    : data(gamedata)
    {}

    bool operator() (const Value &left, const Value &right) {
        if (left.type != right.type) return left.type < right.type;
        if (left.type == Value::String) {
            return data.getString(left.value).text < data.getString(right.value).text;
        } else {
            return left.value < right.value;
        }
    }
private:
    const GameData &data;
};
void GameData::sortList(const Value &listId) {
    ListDef &theList = getList(listId.value);
    ListItemSorter sorter(*this);
    std::sort(theList.items.begin(), theList.items.end(), sorter);
}
