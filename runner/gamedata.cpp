#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include "gamedata.h"
#include "textutil.h"

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
    for (const auto &def : objects)  if (def.second) delete def.second;
    for (const auto &def : lists)    if (def.second) delete def.second;
    for (const auto &def : maps)     if (def.second) delete def.second;
    for (const auto &def : strings)  if (def.second) delete def.second;
}

const StringDef& GameData::getString(int index) const {
    const auto &def = strings.find(index);
    if (def == strings.end()) {
        throw GameBadReference("Tried to access invalid string number "
                        + std::to_string(index));
    }
    return *def->second;
}
StringDef& GameData::getString(int index) {
    auto def = strings.find(index);
    if (def == strings.end()) {
        throw GameBadReference("Tried to access invalid string number "
                        + std::to_string(index));
    }
    return *def->second;
}
const ListDef& GameData::getList(int index) const {
    const auto &def = lists.find(index);
    if (def == lists.end()) {
        throw GameBadReference("Tried to access invalid list number "
                        + std::to_string(index));
    }
    return *def->second;
}
ListDef& GameData::getList(int index) {
    auto def = lists.find(index);
    if (def == lists.end()) {
        throw GameBadReference("Tried to access invalid list number "
                        + std::to_string(index));
    }
    return *def->second;
}
const MapDef& GameData::getMap(int index) const {
    const auto &def = maps.find(index);
    if (def == maps.end()) {
        throw GameBadReference("Tried to access invalid map number "
                        + std::to_string(index));
    }
    return *def->second;
}
MapDef& GameData::getMap(int index) {
    auto def = maps.find(index);
    if (def == maps.end()) {
        throw GameBadReference("Tried to access invalid map number "
                        + std::to_string(index));
    }
    return *def->second;
}
const ObjectDef& GameData::getObject(int index) const {
    const auto &def = objects.find(index);
    if (def == objects.end()) {
        throw GameBadReference("Tried to access invalid object number "
                        + std::to_string(index));
    }
    return *def->second;
}
ObjectDef& GameData::getObject(int index) {
    auto def = objects.find(index);
    if (def == objects.end()) {
        throw GameBadReference("Tried to access invalid object number "
                        + std::to_string(index));
    }
    return *def->second;
}
const FunctionDef& GameData::getFunction(int index) const {
    const auto &def = functions.find(index);
    if (def == functions.end()) {
        throw GameBadReference("Tried to access invalid function number "
                        + std::to_string(index));
    }
    return def->second;
}
FunctionDef& GameData::getFunction(int index) {
    auto def = functions.find(index);
    if (def == functions.end()) {
        throw GameBadReference("Tried to access invalid funtion number "
                        + std::to_string(index));
    }
    return def->second;
}

std::string NO_SUCH_VOCAB("INVALID VOCAB");
const std::string& GameData::getVocab(int index) const {
    if (index < 0 || index >= static_cast<int>(vocab.size())) {
        NO_SUCH_VOCAB = "INVALID VOCAB " + std::to_string(index);
        return NO_SUCH_VOCAB;
    }
    return vocab[index];
}
int GameData::getVocab(const std::string &text) const {
    for (unsigned i = 0; i < vocab.size(); ++i) {
        if (vocab[i] == text) return i;
    }
    return -1;
}

int GameData::collectGarbage() {
    // clear existing marks
    for (auto &def : objects) if (def.second)  def.second->gcMark = false;
    for (auto &def : lists)   if (def.second)  def.second->gcMark = false;
    for (auto &def : maps)    if (def.second)  def.second->gcMark = false;
    for (auto &def : strings) if (def.second)  def.second->gcMark = false;

    // mark objects
    for (auto &def : objects)   if (def.second && def.second->isStatic) mark(*def.second);
    for (auto &def : lists)     if (def.second && def.second->isStatic) mark(*def.second);
    for (auto &def : maps)      if (def.second && def.second->isStatic) mark(*def.second);
    for (auto &def : strings)   if (def.second && def.second->isStatic) mark(*def.second);
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
    for (auto iter = objects.begin(); iter != objects.end(); ) {
        if (!iter->second || !iter->second->gcMark) {
            if (iter->second) delete iter->second;
            iter = objects.erase(iter);
            ++collectionCount;
        } else {
            ++iter;
        }
    }

    for (auto iter = lists.begin(); iter != lists.end(); ) {
        if (!iter->second || !iter->second->gcMark) {
            if (iter->second) delete iter->second;
            iter = lists.erase(iter);
            ++collectionCount;
        } else {
            ++iter;
        }
    }
    for (auto iter = maps.begin(); iter != maps.end(); ) {
        if (!iter->second || !iter->second->gcMark) {
            if (iter->second) delete iter->second;
            iter = maps.erase(iter);
            ++collectionCount;
        } else {
            ++iter;
        }
    }
    for (auto iter = strings.begin(); iter != strings.end(); ) {
        if (!iter->second || !iter->second->gcMark) {
            if (iter->second) delete iter->second;
            iter = strings.erase(iter);
            ++collectionCount;
        } else {
            ++iter;
        }
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
            case Value::Any:
            case Value::None:
            case Value::Integer:
            case Value::Function:
            case Value::Property:
            case Value::TypeId:
            case Value::LocalVar:
            case Value::JumpTarget:
            case Value::Vocab:
            case Value::VarRef:
                break;
        }
    } catch (const GameBadReference&) {
        // do nothing; no need to mark non-existant objects
    }
}


std::string GameData::getSource(const Value &value) {
    std::string text;
    const DataItem *item = nullptr;

    switch(value.type) {
        case Value::Any:
        case Value::None:
        case Value::Integer:
        case Value::Property:
        case Value::String:
        case Value::LocalVar:
        case Value::VarRef:
        case Value::Vocab:
        case Value::JumpTarget:
        case Value::TypeId:
            return "primitive";
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

    if (!item || item->srcFile == -1) return "no debug info";
    if (item->srcFile == ORIGIN_DYNAMIC) return "dynamic";

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
            newDef->ident = nextList;
            ++nextList;
            newDef->srcFile = newDef->srcLine = newDef->srcName = ORIGIN_DYNAMIC;
            lists.insert(std::make_pair(newDef->ident, newDef));
            return Value(Value::List, newDef->ident);
        }
        case Value::Map: {
            MapDef *newDef = new MapDef;
            newDef->ident = nextMap;
            ++nextMap;
            newDef->srcFile = newDef->srcLine = newDef->srcName = ORIGIN_DYNAMIC;
            maps.insert(std::make_pair(newDef->ident, newDef));
            return Value(Value::Map, newDef->ident);
        }
        case Value::Object: {
            ObjectDef *newDef = new ObjectDef;
            newDef->ident = nextObject;
            ++nextObject;
            newDef->srcFile = newDef->srcLine = newDef->srcName = ORIGIN_DYNAMIC;
            objects.insert(std::make_pair(newDef->ident, newDef));
            return Value(Value::Object, newDef->ident);
        }
        case Value::String: {
            StringDef *newDef = new StringDef;
            newDef->ident = nextString;
            ++nextString;
            newDef->srcFile = newDef->srcLine = newDef->srcName = ORIGIN_DYNAMIC;
            strings.insert(std::make_pair(newDef->ident, newDef));
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
        case Value::Object: {
            const ObjectDef &def = getObject(what.value);
            return def.isStatic;
        }
        case Value::List: {
            const ListDef &def = getList(what.value);
            return def.isStatic;
        }
        case Value::Map: {
            const MapDef &def = getMap(what.value);
            return def.isStatic;
        }
        case Value::String: {
            const StringDef &def = getString(what.value);
            return def.isStatic;
        }
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
            case Value::Vocab:      return what.value > 0 && what.value < static_cast<int>(vocab.size());
            default:                return true;
        }
    } catch (const GameBadReference&) {
        return false;
    }
    return true;
}

void GameData::stringAppend(const Value &stringId, const Value &toAppend, bool wantUpperFirst) {
    stringId.requireType(Value::String);
    std::string newText = asString(toAppend);
    StringDef &strDef = getString(stringId.value);
    if (wantUpperFirst) upperFirst(newText);
    strDef.text += newText;
    normalize(strDef.text);
}

std::string GameData::asString(const Value &value) {
    switch(value.type) {
        case Value::String: {
            const StringDef &strDef = getString(value.value);
            return strDef.text;
        }
        case Value::Vocab: {
            return getVocab(value.value);
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
