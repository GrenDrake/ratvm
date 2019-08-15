#include <iostream>
#include <string>
#include <string.h>
#include "gamedata.h"
#include "io.h"

int main(int argc, char *argv[]) {
    std::string gameFile;
    bool doDump = false;
    bool doSilent = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0) {
            std::cerr << "USAGE: ./run [options] [game file]\n";
            std::cerr << "    -version   Display version data then quit.\n";
            std::cerr << "    -dump      Dump game data then quit.\n";
            std::cerr << "    -quick     Run initial game function then quit.\n";
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-version") == 0) {
            std::cerr << "Console Runner QuollVM, V1.0\n";
            return 0;
        } else if (strcmp(argv[i], "-dump") == 0) {
            doDump = true;
        } else if (strcmp(argv[i], "-silent") == 0) {
            doSilent = true;
        } else if (argv[i][0] == '-') {
            std::cerr << "Unrecognized option " << argv[i] << ".\n";
            return 1;
        } else if (gameFile.empty()) {
            gameFile = argv[i];
        } else {
            std::cerr << "Only one game file may be specified.\n";
            return 1;
        }
    }
    if (gameFile.empty()) gameFile = "game.qvm";


    GameData data;
    data.load(gameFile);
    if (!data.gameLoaded) return 1;

    if (doDump) {
        data.dump();
        return 0;
    }

    for (int i = 0; i < INFO_COUNT; ++i) {
        data.infoText[i] = "";
    }
    data.infoText[INFO_TITLE] = gameFile;
    try {
        gameloop(data, doSilent);
    } catch (GameError &e) {
        std::cerr << "\n" << IO::setFG(IO::Red);
        std::cerr << "RUNTIME ERROR:";
        std::cerr << IO::normal();
        std::cerr << ' ' << e.what() << '\n';
        std::cerr << "CALL STACK:\n";
        if (data.callStack.isEmpty()) {
            std::cerr << "    EMPTY\n";
        } else {
            for (int i = data.callStack.size() - 1; i >= 0 ; --i) {
                const gtCallStack::Frame &frame = data.callStack[i];
                FunctionDef &fdef = data.getFunction(frame.functionId);
                std::cerr << "    ";
                if (fdef.srcName >= 0) {
                    std::cerr << data.getString(fdef.srcName).text;
                } else {
                    std::cerr << "(no debug info)";
                }
                std::cerr << " #" << frame.functionId << ' ';
                if (fdef.srcFile >= 0) {
                    std::cerr << '(' << data.getString(fdef.srcFile).text;
                    if (fdef.srcLine >= 0) {
                        std::cerr << ':' << fdef.srcLine;
                    }
                    std::cerr << ')';
                }
                std::cerr << '\n';

                std::cerr << "        LOCAL:";
                for (const Value &v : frame.stack.argList) {
                    std::cerr << ' ' << v;
                }
                std::cerr << '\n';
                std::cerr << "        STACK:";
                for (const Value &v : frame.stack.mValues) {
                    std::cerr << ' ' << v;
                }
                std::cerr << '\n';
            }
        }
        return 1;
    }
    std::cout << IO::normal();
    return 0;
}