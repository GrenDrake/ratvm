/* **************************************************************************
 * Definitions for the GameData class
 *
 * Part of GTRPE by Gren Drake
 * **************************************************************************/
#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <sstream>

#include "gamedata.h"
#include "symboltable.h"
#include "builderror.h"
#include "origin.h"
#include "token.h"

FunctionDef::~FunctionDef() {
    for (AsmLine *line : asmCode) {
        delete line;
    }
}

void FunctionDef::addLabel(const Origin &origin, const std::string &label) {
    asmCode.push_back(new AsmLabel(origin, label));
}

void FunctionDef::addOpcode(const Origin &origin, int opcode) {
    asmCode.push_back(new AsmOpcode(origin, opcode));
}

void FunctionDef::addValue(const Origin &origin, const Value &value) {
    asmCode.push_back(new AsmValue(origin, value));
}

void FunctionDef::addLocal(const std::string &name, Value::Type type, bool alwaysUsed) {
    if (alwaysUsed) {
        locals.push_back(LocalDef{ name, type, 1 });
    } else {
        locals.push_back(LocalDef{ name, type, 0 });
    }
}
const LocalDef* FunctionDef::getLocal(int position) const {
    if (position < 0 || position >= static_cast<int>(locals.size())) {
        return nullptr;
    }
    return &locals[position];
}
LocalDef* FunctionDef::getLocal(int position) {
    if (position < 0 || position >= static_cast<int>(locals.size())) {
        return nullptr;
    }
    return &locals[position];
}
const LocalDef* FunctionDef::getLocal(const std::string &name) const {
    for (const LocalDef &def : locals) {
        if (def.name == name) return &def;
    }
    return nullptr;
}
LocalDef* FunctionDef::getLocal(const std::string &name) {
    for (LocalDef &def : locals) {
        if (def.name == name) return &def;
    }
    return nullptr;
}
int FunctionDef::getLocalNumber(const std::string &name) const {
    int n = 0;
    for (const LocalDef &def : locals) {
        if (def.name == name) return n;
        ++n;
    }
    return -1;
}

GameData::GameData()
: errorCount(0), vocabStart(0), stringsStart(0), listsStart(0), mapsStart(0), objectsStart(0),
  functionsStart(0), bytecodeStart(0), fileEnd(0),
  nextAnonymousId(firstAnonymousId) {
    objects.push_back(nullptr);
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

unsigned GameData::getStringId(std::string name) {
    normalize(name);
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
        if (!object) continue;
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

int GameData::checkObjectIdents() {
    const unsigned pIdent = getPropertyId("ident");
    const unsigned pSave = getPropertyId("save");
    const unsigned pLoad = getPropertyId("load");

    std::map<int, GameObject*> usedIdents;
    int nextIdent = -1;

    for (GameObject *object : objects) {
        if (!object) continue;
        const GameProperty *ident = object->getProperty(pIdent);
        const GameProperty *save = object->getProperty(pSave);
        const GameProperty *load = object->getProperty(pLoad);

        if (ident) {
            if (ident->value.type != Value::Integer || ident->value.value <= 0) {
                addError(object->origin, ErrorMsg::Error, "Object ident property must positive integer.");
            } else {
                auto existingObject = usedIdents.find(ident->value.value);
                if (existingObject != usedIdents.end()) {
                    std::stringstream ss;
                    ss << "Object ident " << ident->value.value << " already in use by object \"";
                    ss << existingObject->second->name << "\" @ " << existingObject->second->origin << '.';
                    addError(object->origin, ErrorMsg::Error, ss.str());
                } else {
                    usedIdents.insert(std::make_pair(ident->value.value, object));
                    if (ident->value.value >= nextIdent) {
                        nextIdent = ident->value.value + 1;
                    }
                }
            }
        }

        if (!ident && save) {
            addError(object->origin, ErrorMsg::Error, "Object has save property but no ident property.");
        }
        if (!ident && load) {
            addError(object->origin, ErrorMsg::Error, "Object has load property but no ident property.");
        }
        if (!load && save) {
            addError(object->origin, ErrorMsg::Error, "Object has save property but no load property.");
        }
        if (!save && load) {
            addError(object->origin, ErrorMsg::Error, "Object has load property but no save property.");
        }
        if (load && load->value.type != Value::Function) {
            addError(object->origin, ErrorMsg::Error, "Object load property must be function.");
        }
        if (save && save->value.type != Value::Function) {
            addError(object->origin, ErrorMsg::Error, "Object save property must be function.");
        }
    }

    return nextIdent < 0 ? 1 : nextIdent;
}

void GameData::addError(const Origin &origin, ErrorMsg::Type type, const std::string &text) {
    errors.push_back(ErrorMsg{type, origin, text});
    if (type != ErrorMsg::Warning) ++errorCount;
}

bool GameData::hasErrors() const {
    return errorCount > 0;
}

void GameData::addVocab(const std::string &word) {
    if (getVocabNumber(word) < 0) {
        vocab.push_back(word);
    }
}

int GameData::getVocabNumber(const std::string &word) const {
    int count = 0;
    for (const std::string &v : vocab) {
        if (v == word) return count;
        ++count;
    }
    return -1;
}

void GameData::sortVocab() {
    std::sort(vocab.begin(), vocab.end());
}

GameObject* GameData::objectById(int ident) {
    for (GameObject *o : objects) {
        if (o && o->globalId == ident) return o;
    }
    return nullptr;
}

GameList* GameData::listById(int ident) {
    for (GameList *o : lists) {
        if (o && o->globalId == ident) return o;
    }
    return nullptr;
}

GameMap* GameData::mapById(int ident) {
    for (GameMap *o : maps) {
        if (o && o->globalId == ident) return o;
    }
    return nullptr;
}

FunctionDef* GameData::functionById(int ident) {
    for (FunctionDef *o : functions) {
        if (o && o->globalId == ident) return o;
    }
    return nullptr;
}

FlagSet* GameData::flagSetById(int ident) {
    for (FlagSet &o : flagsets) {
        if (o.globalId == ident) return &o;
    }
    return nullptr;
}


std::ostream& operator<<(std::ostream &out, const ErrorMsg::Type &type) {
    switch(type) {
        case ErrorMsg::Fatal:   out << "fatal";     break;
        case ErrorMsg::Warning: out << "warning";   break;
        case ErrorMsg::Error:   out << "error";     break;
    }
    return out;
}
