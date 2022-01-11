/* **************************************************************************
 * General Class Definitions
 *
 * This file contains definitions and close utilities for several small utility
 * classes used by the compiler.
 *
 * Part of GTRPE by Gren Drake
 * **************************************************************************/

#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "gamedata.h"
#include "build.h"
#include "builderror.h"
#include "token.h"

const char ansiEscape = 0x1B;

void dump_errors(GameData &gamedata, bool useAnsiEscapes);

int main(int argc, char *argv[]) {
    std::vector<std::string> sourceFiles;
    std::string outputFile = "game.rvm";
    bool dump_tokens = false;
    bool dump_data = false;
    bool dump_bytecode = false;
    bool dump_functionHeaders = false;
    bool dump_functionBytecode = false;
    bool dump_strings = false;
    bool dump_asmCode = false;
    bool dump_irFlag = false;
    bool dump_objTree = false;
    bool skipIdentCheck = false;
    bool useAnsiEscapes = true;
    bool showFiles = false;
    bool showNextIdent = false;

    auto runStart = std::chrono::system_clock::now();

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
        } else if (strcmp(argv[i], "-objtree") == 0) {
            dump_objTree = true;
        } else if (strcmp(argv[i], "-skip-ident-check") == 0) {
            skipIdentCheck = true;

        } else if (strcmp(argv[i], "-color") == 0) {
            useAnsiEscapes = true;
        } else if (strcmp(argv[i], "-no-color") == 0) {
            useAnsiEscapes = false;
        } else if (strcmp(argv[i], "-show-files") == 0) {
            showFiles = true;
        } else if (strcmp(argv[i], "-show-next-ident") == 0) {
            showNextIdent = true;

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
            if (showFiles) {
                std::cerr << "[including file " << file << ".]\n";
            }
            std::vector<Token> newTokens = lex_file(gamedata, file);
            tokens.insert(tokens.end(), newTokens.begin(), newTokens.end());
        }
        if (gamedata.hasErrors()) { dump_errors(gamedata, useAnsiEscapes); return 1; }
        gamedata.sortVocab();
        parse_tokens(gamedata, tokens);
        if (gamedata.hasErrors()) { dump_errors(gamedata, useAnsiEscapes); return 1; }
        translate_symbols(gamedata);
        if (gamedata.hasErrors()) { dump_errors(gamedata, useAnsiEscapes); return 1; }
        gamedata.organize();
        if (!skipIdentCheck) {
            nextIdent = gamedata.checkObjectIdents();
        }
        if (gamedata.hasErrors()) { dump_errors(gamedata, useAnsiEscapes); return 1; }
        parse_functions(gamedata);
        if (gamedata.hasErrors()) { dump_errors(gamedata, useAnsiEscapes); return 1; }
        generate(gamedata, outputFile);
        if (gamedata.hasErrors()) { dump_errors(gamedata, useAnsiEscapes); return 1; }
    } catch (BuildError &e) {
        std::cerr << "Error: " << e.getMessage() << '\n';
        return 1;
    }

    if (showNextIdent && !skipIdentCheck) {
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
    if (dump_objTree) {
        std::ofstream objtreeFile("objtree.dot");
        dump_objtree(gamedata, objtreeFile);
    }

    gamedata.symbols.markUsed("TITLE");
    gamedata.symbols.markUsed("AUTHOR");
    gamedata.symbols.markUsed("VERSION");
    gamedata.symbols.markUsed("GAMEID");
    gamedata.symbols.markUsed("main");
    for (const SymbolDef &s : gamedata.symbols.symbols) {
        if (s.uses == 0) {
            gamedata.addError(s.origin, ErrorMsg::Warning, "Symbol " + s.name + " declared but never used.");
        }
    }

    if (!gamedata.errors.empty()) {
        dump_errors(gamedata, useAnsiEscapes);
    }
    if (!gamedata.hasErrors()) {
        auto runEnd = std::chrono::system_clock::now();
        auto buildTimeRaw = std::chrono::duration_cast<std::chrono::milliseconds>(runEnd - runStart);
        int buildTimeMS = buildTimeRaw.count();
        int buildTimeS = buildTimeMS / 1000;
        buildTimeMS -= buildTimeS * 1000;
        if (useAnsiEscapes) std::cerr << ansiEscape << "[92m";
        std::cerr << "[Created gamefile " << outputFile << " in ";
        std::cerr << buildTimeS << ".";
        std::cerr << std::setfill('0') << std::setw(3) << buildTimeMS << " ms.]\n";
        if (useAnsiEscapes) std::cerr << ansiEscape << "[0m";
    }

    return 0;
}

void dump_errors(GameData &gamedata, bool useAnsiEscapes) {
    int warnCount = gamedata.errors.size() - gamedata.errorCount;

    for (const ErrorMsg &error : gamedata.errors) {
        std::cerr << error.origin << ": ";
        if (useAnsiEscapes) {
            switch(error.type) {
                case ErrorMsg::Error:   std::cerr << ansiEscape << "[91m"; break;
                case ErrorMsg::Warning: std::cerr << ansiEscape << "[93m"; break;
                case ErrorMsg::Fatal:   std::cerr << ansiEscape << "[95m"; break;
            }
        }
        std::cerr << error.type;
        if (useAnsiEscapes) std::cerr << ansiEscape << "[0m";
        std::cerr << ": " << error.message << '\n';
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