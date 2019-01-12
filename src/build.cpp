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
    bool dump_bytecode = false;
    bool dump_functionBytecode = false;
    int next_filename = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-data") == 0) {
            dump_data = true;
        } else if (strcmp(argv[i], "-full-bytecode") == 0) {
            dump_bytecode = true;
        } else if (strcmp(argv[i], "-bytecode") == 0) {
            dump_functionBytecode = true;
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

    try {
        tokens = lex_file(gamedata, sourceFile);
        if (!gamedata.errors.empty()) { dump_errors(gamedata); return 1; }
        preprocess_tokens(gamedata, tokens);
        if (!gamedata.errors.empty()) { dump_errors(gamedata); return 1; }
        parse_tokens(gamedata, tokens);
        if (!gamedata.errors.empty()) { dump_errors(gamedata); return 1; }
        add_default_constants(gamedata);
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
        dump_gamedata(gamedata, dataFile, true, dump_functionBytecode, dump_bytecode);
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