#ifndef TEXTUTIL_H
#define TEXTUTIL_H

#include <string>

enum class IntParseError {
    OK,
    INVALID,
    OUT_OF_RANGE
};

bool c_isspace(int c);
bool isValidIdentifier(int c);
void normalize(std::string &s);
bool validSymbol(const std::string &name);
std::ostream& operator<<(std::ostream &out, const IntParseError &err);
IntParseError parseAsInt(std::string text, int &result);

#endif
