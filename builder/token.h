/* **************************************************************************
 * Token Class Definitions
 *
 * The Token class is used to store information about the individual tokens
 * that the lexer identifies.
 *
 * Part of GTRPE by Gren Drake
 * **************************************************************************/

#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include "origin.h"

class Token {
public:
    enum Type {
        Identifier,
        String,
        Vocab,
        Integer,
        Property,

        OpenBrace,
        CloseBrace,
        OpenSquare,
        CloseSquare,
        OpenParan,
        CloseParan,

        Semicolon,
        Colon,
        Indirection,
        AtSymbol,
        EndOfFile,
    };

    Token()
    : type(Integer), value(0)
    { }
    Token(const Origin &origin, Type type)
    : origin(origin), type(type), value(0)
    { }
    Token(const Origin &origin, Type type, const std::string &text)
    : origin(origin), type(type), text(text), value(0)
    { }
    Token(const Origin &origin, Type type, int value)
    : origin(origin), type(type), value(value)
    { }
    Token(const Origin &origin, Type type, const std::string &text, int value)
    : origin(origin), type(type), text(text), value(value)
    { }

    Origin origin;
    Type type;
    std::string text;
    int value;
};

std::string readFile(const std::string &file);
std::ostream& operator<<(std::ostream &out, const Token::Type &type);
std::ostream& operator<<(std::ostream &out, const Token &token);

#endif
