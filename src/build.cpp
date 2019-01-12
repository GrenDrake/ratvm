/* **************************************************************************
 * General Class Definitions
 *
 * This file contains definitions and close utilities for several small utility
 * classes used by the compiler.
 *
 * Part of GTRPE by Gren Drake
 * **************************************************************************/

#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "gamedata.h"
#include "build.h"
#include "builderror.h"
#include "token.h"

void dump_errors(GameData &gamedata);

int main(int argc, char *argv[]) {
    std::string sourceFile = "source.src";
    std::string outputFile = "game.bin";
    bool dump_tokens = false;
    bool dump_data = false;
    int next_filename = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-data") == 0) {
            dump_data = true;
        } else if (strcmp(argv[i], "-tokens") == 0) {
            dump_tokens = true;
        } else if (next_filename < 2) {
            if (next_filename == 0) {
                sourceFile = argv[i];
            } else {
                outputFile = argv[i];
            }
            ++next_filename;
        } else {
            fprintf(stderr, "Unexpected argument %s\n", argv[i]);
            return 1;
        }
    }

    std::vector<Token> tokens;
    GameData gamedata;
    gamedata.symbols.add(SymbolDef(Origin(), "None",            Value{Value::Integer, 0}));
    gamedata.symbols.add(SymbolDef(Origin(), "Integer",         Value{Value::Integer, 1}));
    gamedata.symbols.add(SymbolDef(Origin(), "String",          Value{Value::Integer, 2}));
    gamedata.symbols.add(SymbolDef(Origin(), "List",            Value{Value::Integer, 3}));
    gamedata.symbols.add(SymbolDef(Origin(), "Map",             Value{Value::Integer, 4}));
    gamedata.symbols.add(SymbolDef(Origin(), "Function",        Value{Value::Integer, 5}));
    gamedata.symbols.add(SymbolDef(Origin(), "Object",          Value{Value::Integer, 6}));
    gamedata.symbols.add(SymbolDef(Origin(), "Property",        Value{Value::Integer, 7}));
    gamedata.symbols.add(SymbolDef(Origin(), "Label",           Value{Value::Integer, 7}));
    gamedata.symbols.add(SymbolDef(Origin(), "InfobarLeft",     Value{Value::Integer, 0}));
    gamedata.symbols.add(SymbolDef(Origin(), "InfobarRight",    Value{Value::Integer, 1}));
    gamedata.symbols.add(SymbolDef(Origin(), "InfobarFooter",   Value{Value::Integer, 2}));
    gamedata.symbols.add(SymbolDef(Origin(), "InfobarTitle",    Value{Value::Integer, 3}));
    gamedata.symbols.add(SymbolDef(Origin(), "none",            Value{Value::None, 0}));

    try {
        tokens = lex_file(gamedata, sourceFile);
        if (!gamedata.errors.empty()) { dump_errors(gamedata); return 1; }
        preprocess_tokens(gamedata, tokens);
        if (!gamedata.errors.empty()) { dump_errors(gamedata); return 1; }
        parse_tokens(gamedata, tokens);
        if (!gamedata.errors.empty()) { dump_errors(gamedata); return 1; }
        translate_symbols(gamedata);
        if (!gamedata.errors.empty()) { dump_errors(gamedata); return 1; }
        gamedata.organize();
        parse_functions(gamedata);
        if (!gamedata.errors.empty()) { dump_errors(gamedata); return 1; }
        generate(gamedata, outputFile);
        if (!gamedata.errors.empty()) { dump_errors(gamedata); return 1; }
    } catch (BuildError &e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    if (dump_data) {
        std::ofstream dataFile("data.txt");
        dump_gamedata(gamedata, dataFile);
    }
    if (dump_tokens) {
        std::ofstream tokenFile("tokens.txt");
        dump_token_list(tokens, tokenFile);
    }

    return 0;
}

void dump_errors(GameData &gamedata) {
    for (const Error &error : gamedata.errors) {
        std::cerr << error.origin << ' ' << error.message << '\n';
    }
    std::cerr << '[' << gamedata.errors.size() << " errors occured.]\n";
}