#ifndef PARSESTATE_H
#define PARSESTATE_H

#include <vector>
#include "token.h"

class GameData;

struct ParseState {
    bool at_end() const;
    bool eof() const;
    const Token* here() const;
    const Token* next();
    const Token* peek() const;
    bool matches(Token::Type type) const;
    bool matches(const std::string &text) const;
    void require(Token::Type type) const;
    void skip(Token::Type type);
    void skip(const std::string &text);
    void skipTo(Token::Type type);

    GameData &gamedata;
    const std::vector<Token> &tokens;
    std::vector<Token>::const_iterator current_token;
};

#endif
