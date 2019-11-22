#ifndef TEXTUTIL_H
#define TEXTUTIL_H

#include <string>

bool c_isspace(int c);
void normalize(std::string &s);
void upperFirst(std::string &s);
std::string codepointToString(int cp);

#endif
