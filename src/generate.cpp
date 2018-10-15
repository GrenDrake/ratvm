/* **************************************************************************
 * Generate game binary data file
 *
 * Part of GTRPE by Gren Drake
 * **************************************************************************/

#include <cctype>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "builderror.h"
#include "gamedata.h"
#include "build.h"
#include "origin.h"
#include "token.h"

const int FILETYPE_ID = 0x47505254;

///////////////////////////////////////////////////////////////////////////////
// Functions to handle writing fixed-size data
void write_32(std::ostream &out, uint32_t value) {
    out.write(reinterpret_cast<char*>(&value), sizeof(value));
}

void write_16(std::ostream &out, uint16_t value) {
    out.write(reinterpret_cast<char*>(&value), sizeof(value));
}

void write_8(std::ostream &out, uint8_t value) {
    out.write(reinterpret_cast<char*>(&value), sizeof(value));
}

void write_str(std::ostream &out, const std::string &text) {
    write_16(out, text.size());
    for (char c : text) {
        write_8(out, c);
    }
}


///////////////////////////////////////////////////////////////////////////////
// Generate the output gamefile
void generate(GameData &gamedata, const std::string &outputFile) {
    std::cerr << "[Creating gamefile ~" << outputFile << "~.]\n";

    std::ofstream out(outputFile);
    if (!out) {
        std::cerr << "Failed to create output file.\n";
        return;
    }
    // write header
    write_32(out, FILETYPE_ID);
    write_32(out, 0);           // format version
    FunctionDef *mainFunction = gamedata.functionByName("main");
    if (mainFunction) {
        write_32(out, mainFunction->ident);
    } else {
        throw BuildError("Function ~main~ not defined.");
    }

    // write strings section
    std::cout << "GenStrings " << out.tellp() << '\n';
    write_32(out, gamedata.stringTable.size());
    for (const std::string &string : gamedata.stringTable) {
        write_str(out, string);
    }

    // write lists section
    std::cout << "GenLists " << out.tellp() << '\n';
    write_32(out, gamedata.lists.size() - 1);
    for (unsigned i = 0; i < gamedata.lists.size(); ++i) {
        const GameList *list = gamedata.lists[i];
        if (list == nullptr) continue;
        write_32(out, i);
        write_16(out, list->items.size());
        for (const Value &value : list->items) {
            write_8(out, value.type);
            write_32(out, value.value);
        }
    }

    // write maps section
    std::cout << "GenMaps " << out.tellp() << '\n';
    write_32(out, gamedata.maps.size() - 1);
    for (unsigned i = 0; i < gamedata.maps.size(); ++i) {
        const GameMap *map = gamedata.maps[i];
        if (map == nullptr) continue;
        write_32(out, i);
        write_16(out, map->rows.size());
        for (const GameMap::MapRow &row : map->rows) {
            write_8(out, row.key.type);
            write_32(out, row.key.value);
            write_8(out, row.value.type);
            write_32(out, row.value.value);
        }
    }

    // write objects section
    std::cout << "GenObj " << out.tellp() << '\n';
    write_32(out, gamedata.objects.size());
    for (const GameObject *object : gamedata.objects) {
        if (object == nullptr) continue;
        write_32(out, object->ident);
        write_16(out, object->properties.size());
        for (const GameProperty &property : object->properties) {
            write_16(out, property.id);
            write_8(out, property.value.type);
            write_32(out, property.value.value);
        }
    }

    // write functions section
    std::cout << "GenFuncs " << out.tellp() << '\n';
    write_32(out, gamedata.functions.size() - 1);
    for (unsigned i = 0; i < gamedata.functions.size(); ++i) {
        const FunctionDef *function = gamedata.functions[i];
        if (function == nullptr) continue;
        write_32(out, function->ident);
        write_16(out, function->argument_count);
        write_16(out, function->local_count);
        write_32(out, function->codePosition);
    }

    // write bytecode section
    std::cout << "ByteCode " << out.tellp() << '\n';
    write_32(out, gamedata.bytecode.size());
    gamedata.bytecode.write(out);

    std::cout << "End of File " << out.tellp() << '\n';
}