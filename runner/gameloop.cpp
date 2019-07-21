#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include "gamedata.h"


int tryAsNumber(const std::string &s) {
    char *endPtr;
    int result = strtol(s.c_str(), &endPtr, 10);
    if (*endPtr == 0) return result;
    return -1;
}

void gameloop(GameData &gamedata, bool doQuick) {
    gamedata.runFunction(gamedata.mainFunction, std::vector<Value>{});
    if (doQuick) return;

    Value nextValue, nextExtra;
    bool hasNext;
    while (1) {

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
                                nextExtra = option.extra;
                                hasNext = true;
                                break;
                            }
                        }
                    }
                    if (!hasNext && inputText.size() == 1) {
                        // hotkey
                    }
                    break; }
                default:
                    break;
            }
        } while (!hasNext);

        std::vector<Value> argList{nextValue, nextExtra};
        gamedata.runFunction(gamedata.optionFunction, argList);
    }

}


