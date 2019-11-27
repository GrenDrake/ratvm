#include <sstream>
#include <utf8proc.h>
#include "builderror.h"

bool c_isspace(int c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool isValidIdentifier(int c) {
    if (c >= '0' && c <= '9') return true;
    if (c >= 'a' && c <= 'z') return true;
    if (c >= 'A' && c <= 'Z') return true;
    if (c == '_' || c == '-') return true;
    return false;
}

void normalize(std::string &s) {
    const unsigned char *source = reinterpret_cast<const unsigned char*>(s.c_str());
    char *result = reinterpret_cast<char*>(utf8proc_NFC(source));
    s = result;
    free(result);
}

bool validSymbol(const std::string &name) {
    for (char c : name) {
        if (!isValidIdentifier(c)) return false;
    }
    return true;
}
