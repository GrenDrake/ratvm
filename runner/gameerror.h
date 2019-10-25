#ifndef GAMEERROR_H_3474867
#define GAMEERROR_H_3474867

#include <stdexcept>

class GameError : public std::runtime_error {
public:
    GameError(const std::string &msg)
    : std::runtime_error(msg)
    { }
};

class GameBadReference : public GameError {
public:
    GameBadReference(const std::string &msg)
    : GameError(msg)
    { }
};

#endif
