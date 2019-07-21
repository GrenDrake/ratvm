#include <iostream>
#include "gamedata.h"

void dump_string(const std::string &text) {
    for (char c : text) {
        if (c < 32) {
            switch(c) {
                case '\n':  std::cout << "\\n"; break;
                case '\r':  std::cout << "\\r"; break;
                case '\t':  std::cout << "\\t"; break;
                default:    std::cout << "\\" << (int)c; break;
            }
        } else {
            std::cout << c;
        }
    }
}

void GameData::dump() const {
    std::cout << "\n## Strings\n";
    for (unsigned i = 0; i < strings.size(); ++i) {
        std::cout << '[' << i << "] ~";
        dump_string(strings[i].text);
        std::cout << "~\n";
    }

    std::cout << "\n## Lists\n";
    for (unsigned i = 1; i < lists.size(); ++i) {
        std::cout << '[' << i << "] {";
        for (const Value &value : lists[i].items) {
            std::cout << ' ' << value;
        }
        std::cout << " }\n";
    }

    std::cout << "\n## Maps\n";
    for (unsigned i = 1; i < maps.size(); ++i) {
        std::cout << '[' << i << "] {";
        for (const MapDef::Row &row : maps[i].rows) {
            std::cout << " (" << row.key << ", " << row.value << ")";
        }
        std::cout << " }\n";
    }

    std::cout << "\n## Objects\n";
    for (unsigned i = 1; i < objects.size(); ++i) {
        std::cout << '[' << i << "] {";
        for (const auto &property : objects[i].properties) {
            std::cout << " (" << property.first << ", " << property.second << ")";
        }
        std::cout << " }\n";
    }

    std::cout << "\n## Function Headers\n";
    for (unsigned i = 1; i < functions.size(); ++i) {
        std::cout << '[' << i << "] args: ";
        std::cout << functions[i].arg_count << " locals: ";
        std::cout << functions[i].local_count << " position: ";
        std::cout << functions[i].position << "\n";
    }
}
