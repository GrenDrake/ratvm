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

void translate_value(GameData &gamedata, Value &value) {
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
