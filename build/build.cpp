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
    std::vector<std::string> sourceFiles;
    std::string outputFile = "game.qvm";
    bool dump_tokens = false;
    bool dump_data = false;
    bool dump_bytecode = false;
    bool dump_functionHeaders = false;
    bool dump_functionBytecode = false;
    bool dump_strings = false;
    bool dump_asmCode = false;
    bool dump_irFlag = false;
    bool skipIdentCheck = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-data") == 0) {
            dump_data = true;
        } else if (strcmp(argv[i], "-full-bytecode") == 0) {
            dump_bytecode = true;
        } else if (strcmp(argv[i], "-bytecode") == 0) {
            dump_functionBytecode = true;
            dump_functionHeaders = true;
        } else if (strcmp(argv[i], "-tokens") == 0) {
            dump_tokens = true;
        } else if (strcmp(argv[i], "-functions") == 0) {
            dump_functionHeaders = true;
        } else if (strcmp(argv[i], "-asm") == 0) {
            dump_asmCode = true;
        } else if (strcmp(argv[i], "-strings") == 0) {
            dump_strings = true;
        } else if (strcmp(argv[i], "-ir") == 0) {
            dump_irFlag = true;
        } else if (strcmp(argv[i], "-skip-ident-check") == 0) {
            skipIdentCheck = true;
        } else if (strcmp(argv[i], "-o") == 0) {
            ++i;
            if (i >= argc) {
                std::cerr << "-o argument requries name of output file.\n";
                return 1;
            }
            outputFile = argv[i];
        } else if (argv[i][0] == '-') {
            std::cerr << "Unrecognized argument " << argv[i] << ".\n";
            return 1;
        } else {
            sourceFiles.push_back(argv[i]);
        }
    }

    if (sourceFiles.empty()) {
        std::cerr << "No source files specified.\n";
        return 1;
    }

    int nextIdent = -1;
    std::vector<Token> tokens;
    GameData gamedata;
    add_default_constants(gamedata);

    try {
        for (const std::string &file : sourceFiles) {
            std::vector<Token> newTokens = lex_file(gamedata, file);
            tokens.insert(tokens.end(), newTokens.begin(), newTokens.end());
        }
        if (gamedata.hasErrors()) { dump_errors(gamedata); return 1; }
        parse_tokens(gamedata, tokens);
        if (gamedata.hasErrors()) { dump_errors(gamedata); return 1; }
        translate_symbols(gamedata);
        if (gamedata.hasErrors()) { dump_errors(gamedata); return 1; }
        gamedata.organize();
        if (!skipIdentCheck) {
            nextIdent = gamedata.checkObjectIdents();
        }
        if (gamedata.hasErrors()) { dump_errors(gamedata); return 1; }
        parse_functions(gamedata);
        if (gamedata.hasErrors()) { dump_errors(gamedata); return 1; }
        generate(gamedata, outputFile);
        if (gamedata.hasErrors()) { dump_errors(gamedata); return 1; }
    } catch (BuildError &e) {
        std::cerr << "Error: " << e.getMessage() << '\n';
        return 1;
    }

    if (nextIdent > 0) {
        std::cerr << "[next available ident: " << nextIdent << "]\n";
    }

    if (dump_data) {
        std::ofstream dataFile("data.txt");
        dump_gamedata(gamedata, dataFile);
    }
    if (dump_functionHeaders) {
        std::ofstream functionsFile("functions.txt");
        dump_functions(gamedata, functionsFile, dump_functionBytecode);
    }
    if (dump_asmCode) {
        std::ofstream asmFile("asm.txt");
        dump_asm(gamedata, asmFile);
    }
    if (dump_irFlag) {
        std::ofstream irFile("ir.txt");
        dump_ir(gamedata, irFile);
    }
    if (dump_bytecode) {
        std::ofstream bytecodeFile("bytecode.txt");
        dump_fullBytecode(gamedata, bytecodeFile);
    }
    if (dump_strings) {
        std::ofstream stringFile("strings.txt");
        dump_stringtable(gamedata, stringFile);
    }
    if (dump_tokens) {
        std::ofstream tokenFile("tokens.txt");
        dump_token_list(tokens, tokenFile);
    }

    if (!gamedata.errors.empty()) {
        dump_errors(gamedata);
    }
    return 0;
}

void dump_errors(GameData &gamedata) {
    int warnCount = gamedata.errors.size() - gamedata.errorCount;

    for (const ErrorMsg &error : gamedata.errors) {
        std::cerr << error.origin << ": ";
        std::cerr << error.type << ": ";
        std::cerr << error.message << '\n';
    }
    std::cerr << '[';
    if (gamedata.errorCount > 0) {
        std::cerr << gamedata.errorCount << " error";
        if (gamedata.errorCount > 1) std::cerr << 's';
    }
    if (gamedata.errorCount > 0 && warnCount > 0) {
        std::cerr << " and ";
    }
    if (warnCount > 0) {
        std::cerr << warnCount << " warning";
        if (warnCount > 1) std::cerr << 's';
    }
    std::cerr << " occured.]\n";
}