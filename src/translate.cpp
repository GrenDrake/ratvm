/* **************************************************************************
 * Translate all symbols into their real values
 *
 * Part of GTRPE by Gren Drake
 * **************************************************************************/

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "builderror.h"
#include "gamedata.h"
#include "build.h"

void add_default_constants(GameData &gamedata) {
    gamedata.symbols.add(SymbolDef(Origin(), "None",            Value{Value::Integer, 0}));
    gamedata.symbols.add(SymbolDef(Origin(), "Integer",         Value{Value::Integer, 1}));
    gamedata.symbols.add(SymbolDef(Origin(), "String",          Value{Value::Integer, 2}));
    gamedata.symbols.add(SymbolDef(Origin(), "List",            Value{Value::Integer, 3}));
    gamedata.symbols.add(SymbolDef(Origin(), "Map",             Value{Value::Integer, 4}));
    gamedata.symbols.add(SymbolDef(Origin(), "Function",        Value{Value::Integer, 5}));
    gamedata.symbols.add(SymbolDef(Origin(), "Object",          Value{Value::Integer, 6}));
    gamedata.symbols.add(SymbolDef(Origin(), "Property",        Value{Value::Integer, 7}));
    gamedata.symbols.add(SymbolDef(Origin(), "Stream",          Value{Value::Integer, 8}));
    gamedata.symbols.add(SymbolDef(Origin(), "Label",           Value{Value::Integer, 9}));
    gamedata.symbols.add(SymbolDef(Origin(), "InfobarLeft",     Value{Value::Integer, 0}));
    gamedata.symbols.add(SymbolDef(Origin(), "InfobarRight",    Value{Value::Integer, 1}));
    gamedata.symbols.add(SymbolDef(Origin(), "InfobarFooter",   Value{Value::Integer, 2}));
    gamedata.symbols.add(SymbolDef(Origin(), "InfobarTitle",    Value{Value::Integer, 3}));
    gamedata.symbols.add(SymbolDef(Origin(), "none",            Value{Value::None,    0}));
    gamedata.symbols.add(SymbolDef(Origin(), "saveAllowed",     Value{Value::Integer, 0}));
    gamedata.symbols.add(SymbolDef(Origin(), "true",            Value{Value::Integer, 1}));
    gamedata.symbols.add(SymbolDef(Origin(), "false",           Value{Value::Integer, 0}));
    gamedata.getPropertyId("(invalid)");
    gamedata.getPropertyId("internal_name");
    gamedata.getPropertyId("ident");
    gamedata.getPropertyId("save");
    gamedata.getPropertyId("load");
}

void translate_value(GameData &gamedata, Value &value) {
    if (value.type == Value::FlagSet) {
        value.type = Value::Integer;
        value.value = gamedata.flagsets[value.value].finalValue;
        return;
    }
    if (value.type != Value::Symbol) return;
    const SymbolDef *symbol = gamedata.symbols.get(value.text);
    if (!symbol) {
        std::stringstream ss;
        ss << "Undefined symbol ~" << value.text << "~.";
        throw BuildError(ss.str());
    }
    value.type = symbol->value.type;
    value.value = symbol->value.value;
}

void translate_symbols(GameData &gamedata) {
    for (FlagSet &flagset : gamedata.flagsets) {
        unsigned result = 0;
        for (Value &value : flagset.values) {
            translate_value(gamedata, value);
            if (value.type == Value::Integer) {
                result |= value.value;
            } else {
                gamedata.errors.push_back(Error{flagset.origin, "Flag values must be integers."});
            }
        }
        flagset.finalValue = result;
    }

    for (SymbolDef &symbol : gamedata.symbols.symbols) {
        if (symbol.value.type == Value::FlagSet) {
            translate_value(gamedata, symbol.value);
            if (symbol.value.type != Value::Integer) {
                gamedata.errors.push_back(Error{symbol.origin, "Invalid value in flag set."});
            }
        }
    }


    for (GameObject *object : gamedata.objects) {
        if (object == nullptr) continue;
        for (GameProperty &property : object->properties) {
            translate_value(gamedata, property.value);
        }
    }

    for (GameList *list : gamedata.lists) {
        if (list == nullptr) continue;
        for (Value &value : list->items) {
            translate_value(gamedata, value);
        }
    }

    for (GameMap *map : gamedata.maps) {
        if (map == nullptr) continue;
        for (GameMap::MapRow &row : map->rows) {
            translate_value(gamedata, row.key);
            translate_value(gamedata, row.value);
        }
    }
}
