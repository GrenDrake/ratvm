#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <string>
#include <vector>

#include "origin.h"
#include "value.h"

struct SymbolDef {
    enum Type {
        Object, Integer, String
    };

    SymbolDef(const Origin &origin, const std::string &name, const Value &value, unsigned initialUses = 0)
    : origin(origin), name(name), value(value), uses(initialUses)
    { }

    Origin origin;
    std::string name;
    Value value;
    unsigned uses;
};

class SymbolTable {
public:
    void add(const Origin &definingAt, const SymbolDef &symbol);
    const SymbolDef* get(const std::string &name, bool countsAsUse = false);
    void markUsed(const std::string &name);
    std::vector<SymbolDef> symbols;
};

std::ostream& operator<<(std::ostream &out, const SymbolDef::Type &type);

#endif
