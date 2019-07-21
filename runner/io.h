#ifndef IO_H_357435467
#define IO_H_357435467

#include <string>

namespace IO {
    enum Color {
        Black = 30,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White
    };

    std::string bold() {
        return "\x1B[1m";
    }
    std::string underline() {
        return "\x1B[4m";
    }
    std::string normal() {
        return "\x1B[0m";
    }

    std::string setFG(Color color) {
        return "\x1B[" + std::to_string(static_cast<int>(color)) + "m";
    }
    std::string setBG(Color color) {
        return "\x1B[" + std::to_string(10 + static_cast<int>(color)) + "m";
    }
};

#endif
