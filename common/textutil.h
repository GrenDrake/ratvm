#ifndef TEXTUTIL_H_7854902
#define TEXTUTIL_H_7854902

#include <string>
#include <vector>

enum class IntParseError {
    OK,
    INVALID,
    OUT_OF_RANGE
};

bool c_isspace(int c);
int c_tolower(int c);
bool isValidIdentifier(int c);
void normalize(std::string &s);
IntParseError parseAsInt(std::string text, int &result);
void upperFirst(std::string &s);
bool validSymbol(const std::string &name);
int getFirstCodepoint(const std::string &s);
std::string codepointToString(int cp);
std::string& trim(std::string &text);
std::string trim(const std::string &text);
std::vector<std::string> explodeString(const std::string &s);
std::string &strToLower(std::string &text);

std::ostream& operator<<(std::ostream &out, const IntParseError &err);

#endif
