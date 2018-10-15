#include <ostream>

#include "gamedata.h"

void dump_gamedata(GameData &gamedata, std::ostream &out) {
    for (unsigned i = 0; i < gamedata.propertyNames.size(); ++i) {
        out << "PROPERTY-ID " << i << " " << gamedata.propertyNames[i] << '\n';
    }
    out << '\n';

    for (unsigned i = 0; i < gamedata.stringTable.size(); ++i) {
        out << "STRING " << i << " ~" << gamedata.stringTable[i].substr(0, 60) << "~\n";
    }
    out << '\n';

    for (unsigned i = 0; i < gamedata.lists.size(); ++i) {
        const GameList *list = gamedata.lists[i];
        if (list == nullptr) continue;
        out << "LIST " << list->ident << " @ " << list->origin << "\n   ";
        for (unsigned i = 0; i < list->items.size(); ++i) {
            out << ' ' << list->items[i];
        }
        out << '\n';
    }
    out << '\n';

    for (unsigned i = 0; i < gamedata.maps.size(); ++i) {
        const GameMap *map = gamedata.maps[i];
        if (map == nullptr) continue;
        out << "MAP " << map->ident << " @ " << map->origin << "\n";
        for (unsigned i = 0; i < map->rows.size(); ++i) {
            out << "    " << map->rows[i].key << " => " << map->rows[i].value << '\n';
        }
    }
    out << '\n';

    for (const GameObject *object : gamedata.objects) {
        if (object == nullptr) continue;
        out << "OBJECT " << object->ident << " ";
        if (object->name.empty())   out << "(anonymous)";
        else                        out << object->name;
        out << " @ " << object->origin << "\n";
        for (const GameProperty &prop : object->properties) {
            out << "    " << prop.id << ": " << prop.value << '\n';
        }
    }
    out << '\n';

    for (unsigned i = 0; i < gamedata.functions.size(); ++i) {
        const FunctionDef *function = gamedata.functions[i];
        if (function == nullptr) continue;
        out << "FUNCTION " << function->ident << ' ';
        if (function->name.empty()) out << "(anonymous)";
        else                        out << function->name;
        out << " @ " << function->origin << "\n";
        out << "    ARGUMENTS:";
        for (int i = 0; i < function->argument_count; ++i) {
            out << ' ' << function->local_names[i];
        }
        out << "\n    LOCALS:";
        for (int i = 0; i < function->local_count; ++i) {
            out << ' ' << function->local_names[i + function->argument_count];
        }
        out << "\n    TOKEN COUNT: " << function->tokens.size();
        out << "\n    CODE POSITION: " << function->codePosition << " (0x";
        out << std::hex << function->codePosition << std::dec << ')';
        out << "\n    BYTE CODE:";
        function->code.dump(out, 8);
    }
    out << '\n';

    for (const SymbolDef &symbol : gamedata.symbols.symbols) {
        out << "SYMBOL " << symbol.name << " = " << symbol.value;
        out << " @ " << symbol.origin << '\n';
    }

    out << "\nBYTECODE SEGMENT SIZE: " << gamedata.bytecode.size();
    gamedata.bytecode.dump(out, 0);
}

void dump_token_list(const std::vector<Token> &tokens, std::ostream &out) {
    for (const Token &t : tokens) {
        out << t << "\n";
    }
}
