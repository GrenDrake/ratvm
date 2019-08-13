#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include "gamedata.h"

std::string formatText(const std::string &text);

int tryAsNumber(const std::string &s) {
    char *endPtr;
    int result = strtol(s.c_str(), &endPtr, 10);
    if (*endPtr == 0) return result;
    return -1;
}

void gameloop(GameData &gamedata, bool doQuick) {
    const FunctionDef &funcDef = gamedata.getFunction(gamedata.mainFunction);
    gamedata.callStack.create(funcDef, gamedata.mainFunction);
    gamedata.callStack.getStack().setArgs(std::vector<Value>{Value{Value::None, 0}}, funcDef.arg_count, funcDef.local_count);
    gamedata.callStack.callTop().IP = funcDef.position;

    Value nextValue;
    bool hasNext, hasValue = false;
    while (1) {
        gamedata.textBuffer = "";
        gamedata.options.clear();
        gamedata.resume(hasValue, nextValue);
        hasValue = false;

        std::cout << "\n*** " << gamedata.infoText[INFO_TITLE] << " ***\n";
        std::cout << gamedata.infoText[INFO_LEFT];
        std::cout << " : ";
        std::cout << gamedata.infoText[INFO_RIGHT];
        std::cout << '\n';
        std::cout << formatText(gamedata.textBuffer) << '\n';
        if (!gamedata.infoText[INFO_BOTTOM].empty()) {
            std::cout << "[";
            std::cout << gamedata.infoText[INFO_BOTTOM];
            std::cout << "]\n";
        }


        switch(gamedata.optionType) {
            case OptionType::Choice: {
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
            std::transform(inputText.begin(), inputText.end(), inputText.begin(),
                    [](unsigned char c) -> unsigned char { return std::tolower(c); });
            if (inputText == "quit") {
                std::cout << "\nGoodbye!\n";
                return;
            }

            switch(gamedata.optionType) {
                case OptionType::Choice: {
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


