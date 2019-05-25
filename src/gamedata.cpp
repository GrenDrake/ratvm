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

void FunctionDef::dumpAsm(FunctionDef *function, std::ostream &out) const {
    for (const AsmLine *line : function->asmCode) {
        const AsmLabel *label = dynamic_cast<const AsmLabel*>(line);
        if (label) {
            out << "LABEL " << label->text << '\n';
            continue;
        }

        const AsmOpcode *code = dynamic_cast<const AsmOpcode*>(line);
        if (code) {
            out << "OPCODE " << code->opcode << '\n';
            continue;
        }

        const AsmValue *value = dynamic_cast<const AsmValue*>(line);
        if (value) {
            out << "VALUE " << value->value << '\n';
            continue;
        }
    }
}

GameData::GameData()
: nextAnonymousId(firstAnonymousId) {
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
                errors.push_back(Error{object->origin, "Object ident property must positive integer."});
            } else {
                auto existingObject = usedIdents.find(ident->value.value);
                if (existingObject != usedIdents.end()) {
                    std::stringstream ss;
                    ss << "Object ident " << ident->value.value << " already in use by object \"";
                    ss << existingObject->second->name << "\" @ " << existingObject->second->origin << '.';
                    errors.push_back(Error{object->origin, ss.str()});
                } else {
                    usedIdents.insert(std::make_pair(ident->value.value, object));
                    if (ident->value.value >= nextIdent) {
                        nextIdent = ident->value.value + 1;
                    }
                }
            }
        }

        if (!ident && save) {
            errors.push_back(Error{object->origin, "Object has save property but no ident property."});
        }
        if (!ident && load) {
            errors.push_back(Error{object->origin, "Object has load property but no ident property."});
        }
        if (!load && save) {
            errors.push_back(Error{object->origin, "Object has save property but no load property."});
        }
        if (!save && load) {
            errors.push_back(Error{object->origin, "Object has load property but no save property."});
        }
        if (load && load->value.type != Value::Node) {
            errors.push_back(Error{object->origin, "Object load property must be function."});
        }
        if (save && save->value.type != Value::Node) {
            errors.push_back(Error{object->origin, "Object save property must be function."});
        }
    }

    return nextIdent;
}

unsigned GameData::getSourceFileIndex(const std::string &filename) {
    for (unsigned i = 0; i < sourceFiles.size(); ++i) {
        if (sourceFiles[i] == filename) return i;
    }
    sourceFiles.push_back(filename);
    return sourceFiles.size() - 1;
}

