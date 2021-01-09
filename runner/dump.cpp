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
    for (const auto &def : strings) {
        std::cout << '[' << def.first << ((def.second && def.second->isStatic) ? 's' : ' ') << "] ~";
        if (def.second)     dump_string(def.second->text);
        else                std::cout << "(nullptr)";
        std::cout << "~\n";
    }

    std::cout << "\n## Lists\n";
    for (const auto &def : lists) {
        std::cout << '[' << def.first << ((def.second && def.second->isStatic) ? 's' : ' ') << "] {";
        if (!def.second) {
            std::cout << "(nullptr)";
        } else {
            for (const Value &value : def.second->items) {
                std::cout << ' ' << value;
            }
        }
        std::cout << " }\n";
    }

    std::cout << "\n## Maps\n";
    for (const auto &def : maps) {
        std::cout << '[' << def.first << ((def.second && def.second->isStatic) ? 's' : ' ') << "] {";
        if (!def.second) {
            std::cout << "(nullptr)";
        } else {
            for (const MapDef::Row &row : def.second->rows) {
                std::cout << " (" << row.key << ", " << row.value << ")";
            }
        }
        std::cout << " }\n";
    }

    std::cout << "\n## Objects\n";
    for (const auto &def : objects) {
        std::cout << '[' << def.first << ((def.second && def.second->isStatic) ? 's' : ' ') << "] {";
        if (!def.second) {
            std::cout << "(nullptr)";
        } else {
            for (const auto &property : def.second->properties) {
                std::cout << " (" << property.first << ", " << property.second << ")";
            }
        }
        std::cout << " }\n";
    }

    std::cout << "\n## Function Headers\n";
    for (const auto &def : functions) {
        std::cout << '[' << def.first << "] args: ";
        std::cout << def.second.arg_count << " locals: ";
        std::cout << def.second.local_count << " position: ";
        std::cout << def.second.position << "\n";
    }
}
