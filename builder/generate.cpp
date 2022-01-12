/* **************************************************************************
 * Generate game binary data file
 *
 * Part of GTRPE by Gren Drake
 * **************************************************************************/

#include <cstdint>
#include <ctime>
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
const unsigned char STRING_XOR_KEY = 0x7B;

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
        write_8(out, c ^ STRING_XOR_KEY);
    }
}

void write_symbol(std::ostream &out, const std::string &symbolName, Value::Type requiredType, GameData &gamedata, const std::string &outputFile) {
    const SymbolDef *nameSym = gamedata.symbols.get(symbolName);
    if (nameSym == nullptr) {
        std::stringstream ss;
        ss << "Symbol " << symbolName << " not defined.";
        gamedata.addError(Origin(outputFile,0,0), ErrorMsg::Error, ss.str());
        write_32(out, 0);
    } else if (nameSym->value.type != requiredType) {
        std::stringstream ss;
        ss << "Symbol " << symbolName << " must be " << requiredType << ".";
        gamedata.addError(Origin(outputFile,0,0), ErrorMsg::Error, ss.str());
        write_32(out, 0);
    } else {
        write_32(out, nameSym->value.value);
    }
}


///////////////////////////////////////////////////////////////////////////////
// Generate the output gamefile
void generate(GameData &gamedata, const std::string &outputFile) {
    std::ofstream out(outputFile, std::ios_base::binary);
    if (!out) {
        gamedata.addError(Origin(), ErrorMsg::Fatal, "Failed to create output file.");
        return;
    }
    // write header
    write_32(out, FILETYPE_ID); // 0: magic number
    write_32(out, 0);           // 4: format version
    // 8: main function index
    write_symbol(out, "main", Value::Function, gamedata, outputFile);
    // 12: write gamefile flags
    write_32(out, 0);

    // 16, 20, 24, 28: title, author, version, gameid, and build number
    write_symbol(out, "TITLE",   Value::String,  gamedata, outputFile); // 16: game title
    write_symbol(out, "AUTHOR",  Value::String,  gamedata, outputFile); // 20: game author
    write_symbol(out, "VERSION", Value::Integer, gamedata, outputFile); // 24: game version
    write_symbol(out, "GAMEID",  Value::String,  gamedata, outputFile); // 28: game id
    write_32(out, static_cast<uint32_t>(std::time(nullptr)));           // 32: build number

    // pad to the header out to 64 bytes
    write_32(out, 0);   // 36: padding
    write_32(out, 0);   // 40: padding
    write_32(out, 0);   // 44: padding
    write_32(out, 0);   // 48: padding
    write_32(out, 0);   // 52: padding
    write_32(out, 0);   // 56: padding
    write_32(out, 0);   // 60: padding

    // write strings section
    gamedata.stringsStart = out.tellp();
    write_32(out, gamedata.stringTable.size());
    for (const std::string &string : gamedata.stringTable) {
        write_str(out, string);
    }

    // write dictionary section
    gamedata.vocabStart = out.tellp();
    write_32(out, gamedata.vocab.size());
    for (const std::string &string : gamedata.vocab) {
        write_str(out, string);
    }

    // write lists section
    gamedata.listsStart = out.tellp();
    write_32(out, gamedata.lists.size() - 1);
    for (unsigned i = 0; i < gamedata.lists.size(); ++i) {
        const GameList *list = gamedata.lists[i];
        if (list == nullptr) continue;
        write_32(out, list->origin.fileNameString);
        write_32(out, list->origin.line);
        write_32(out, list->globalId);
        write_16(out, list->items.size());
        for (const Value &value : list->items) {
            write_8(out, value.type);
            write_32(out, value.value);
        }
    }

    // write maps section
    gamedata.mapsStart = out.tellp();
    write_32(out, gamedata.maps.size() - 1);
    for (unsigned i = 0; i < gamedata.maps.size(); ++i) {
        const GameMap *map = gamedata.maps[i];
        if (map == nullptr) continue;
        write_32(out, map->origin.fileNameString);
        write_32(out, map->origin.line);
        write_32(out, map->globalId);
        write_16(out, map->rows.size());
        for (const GameMap::MapRow &row : map->rows) {
            write_8(out, row.key.type);
            write_32(out, row.key.value);
            write_8(out, row.value.type);
            write_32(out, row.value.value);
        }
    }

    // write objects section
    gamedata.objectsStart = out.tellp();
    write_32(out, gamedata.objects.size() - 1);
    for (const GameObject *object : gamedata.objects) {
        if (object == nullptr) continue;
        write_32(out, object->nameString);
        write_32(out, object->origin.fileNameString);
        write_32(out, object->origin.line);
        write_32(out, object->globalId);
        write_32(out, object->parentId);
        write_32(out, object->childId);
        write_32(out, object->siblingId);
        write_16(out, object->properties.size());
        for (const GameProperty &property : object->properties) {
            write_16(out, property.id);
            write_8(out, property.value.type);
            write_32(out, property.value.value);
        }
    }

    // write functions section
    gamedata.functionsStart = out.tellp();
    write_32(out, gamedata.functions.size() - 1);
    for (unsigned i = 0; i < gamedata.functions.size(); ++i) {
        const FunctionDef *function = gamedata.functions[i];
        if (function == nullptr) continue;
        write_32(out, function->nameString);
        write_32(out, function->origin.fileNameString);
        write_32(out, function->origin.line);
        write_32(out, function->globalId);
        write_16(out, function->argument_count);
        write_16(out, function->local_count);
        for (const LocalDef &def : function->locals) {
            write_8(out, static_cast<int>(def.type));
        }
        write_32(out, function->codePosition);
    }

    // write bytecode section
    gamedata.bytecodeStart = out.tellp();
    write_32(out, gamedata.bytecode.size());
    gamedata.bytecode.write(out);

    gamedata.fileEnd = out.tellp();
}