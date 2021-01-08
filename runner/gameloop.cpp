#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include "gamedata.h"
#include "formatter.h"
#include "textutil.h"

int tryAsNumber(const std::string &s) {
    char *endPtr;
    int result = strtol(s.c_str(), &endPtr, 10);
    if (*endPtr == 0) return result;
    return -1;
}

void gameloop(GameData &gamedata, bool doSilent) {
    const FunctionDef &funcDef = gamedata.getFunction(gamedata.mainFunction);
    gamedata.callStack.create(funcDef, gamedata.mainFunction);
    gamedata.callStack.getStack().setArgs(std::vector<Value>{Value{Value::None, 0}}, funcDef.arg_count, funcDef.local_count);
    gamedata.callStack.callTop().IP = funcDef.position;

    int garbageCounter = 0, garbageAmount = 0;
    Value nextValue;
    bool hasNext, hasValue = false, didGarbage = false;
    while (1) {
        ++garbageCounter;
        if (garbageCounter >= GARBAGE_FREQUENCY) {
            garbageAmount = gamedata.collectGarbage();
            garbageCounter = 0;
            didGarbage = true;
        } else didGarbage = false;
        gamedata.textBuffer = "";
        gamedata.options.clear();
        gamedata.instructionCount = 0;
        gamedata.resume(hasValue, nextValue);
        hasValue = false;

        if (!doSilent) {
            std::cout << "\n*** " << gamedata.infoText[INFO_TITLE] << " ***\n";
            std::cout << gamedata.infoText[INFO_LEFT];
            std::cout << " : ";
            std::cout << gamedata.infoText[INFO_RIGHT];
            std::cout << '\n';
            ParseResult formatResult = formatText(gamedata.textBuffer);
            if (!formatResult.errors.empty()) {
                std::cout << "--== ==-- --== ==-- --== ==-- --== ==-- --== ==--\nERRORS OCCURED WHILE PARSING TEXT.\n";
                for (const std::string &s : formatResult.errors) {
                    std::cout << "    " << s << "\n";
                }
                std::cout << "--== ==-- --== ==-- --== ==-- --== ==-- --== ==--\n";
            }
            std::cout << formatResult.finalResult << '\n';
            if (!gamedata.infoText[INFO_BOTTOM].empty()) {
                std::cout << "[";
                std::cout << gamedata.infoText[INFO_BOTTOM];
                std::cout << "]\n";
            }
        }
        if (gamedata.showDebug) {
            std::cout << ":: GC - ";
            if (didGarbage) {
                std::cout << garbageAmount << " collected";
            } else {
                std::cout << "did't run";
            }
            std::cout << " :: " << gamedata.instructionCount << " opcodes executed\n";
        }


        switch(gamedata.optionType) {
            case OptionType::EndOfProgram:
                if (!doSilent) {
                    std::cout << "\nProgram ended. Goodbye!\n";
                }
                return;
            case OptionType::Key:
            case OptionType::Line: {
                if (doSilent) break;
                if (!gamedata.options.empty()) {
                    const GameOption &option = gamedata.options.back();
                    std::cout << '\n' << gamedata.getString(option.strId).text;
                }
                break; }
            case OptionType::Choice: {
                if (doSilent) break;
                std::cout << '\n';
                int index = 1;
                for (GameOption &option : gamedata.options) {
                    if (option.hotkey > 0) {
                        option.hotkey = std::toupper(option.hotkey);
                        std::cout << static_cast<char>(option.hotkey) << ") ";
                        std::cout << gamedata.getString(option.strId).text << '\n';
                    } else {
                        std::cout << index << ") ";
                        std::cout << gamedata.getString(option.strId).text << '\n';
                        option.hotkey = -index;
                        ++index;
                    }
                }
                break; }
            default:
                ;
        }

        hasNext = false;
        do {
            std::cout << "\n> ";
            std::string inputText;
            std::getline(std::cin, inputText);
            strToLower(inputText);
            if (inputText == "quit") {
                if (!doSilent) {
                    std::cout << "\nGoodbye!\n";
                }
                return;
            }

            switch(gamedata.optionType) {
                case OptionType::EndOfProgram:
                    //should never occur
                    return;
                case OptionType::Key: {
                    int c = 0;
                    if (!inputText.empty()) {
                        c = getFirstCodepoint(inputText);
                        if (c >= 'A' && c <= 'Z') c += 32;
                    }
                    nextValue = Value(Value::Integer, c);
                    hasNext = true;
                    hasValue = true;
                    break; }
                case OptionType::Line: {
                    nextValue = gamedata.makeNewString(inputText);
                    hasNext = true;
                    hasValue = true;
                    break; }
                case OptionType::Choice: {
                    if (inputText.empty()) {
                        if (gamedata.options.size() == 1) {
                            nextValue = gamedata.options.front().value;
                            gamedata.setExtra(gamedata.options.front().extra);
                            hasNext = true;
                            hasValue = true;
                        }
                        break;
                    }

                    int optNum = tryAsNumber(inputText);
                    if (optNum >= 0) {
                        // numbered choice
                        optNum = -optNum;
                        for (const GameOption &option : gamedata.options) {
                            if (option.hotkey == optNum) {
                                nextValue = option.value;
                                gamedata.setExtra(option.extra);
                                hasNext = true;
                                hasValue = true;
                                break;
                            }
                        }
                    } else if (inputText.size() == 1) {
                        char key = std::toupper(inputText[0]);
                        // hotkey choice
                        for (const GameOption &option : gamedata.options) {
                            if (option.hotkey == key) {
                                nextValue = option.value;
                                gamedata.setExtra(option.extra);
                                hasNext = true;
                                hasValue = true;
                                break;
                            }
                        }
                    }
                    break; }
                default:
                    std::cerr << "Unknown option type " << static_cast<int>(gamedata.optionType) << '\n';
                    break;
            }
        } while (!hasNext);
    }

}
