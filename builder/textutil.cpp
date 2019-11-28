#include <cstdint>
#include <sstream>
#include <ostream>
#include <utf8proc.h>
#include "textutil.h"
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

std::ostream& operator<<(std::ostream &out, const IntParseError &err) {
    switch(err) {
        case IntParseError::OK:
            out << "OK";
            break;
        case IntParseError::INVALID:
            out << "INVALID";
            break;
        case IntParseError::OUT_OF_RANGE:
            out << "OUT_OF_RANGE";
            break;
    }
    return out;
}

IntParseError parseAsInt(std::string text, int &result) {
    int base = 10;
    if (text.size() >= 2 && text[0] == '0') {
        if (text[1] == 'X' || text[1] == 'x') {
            base = 16;
            text.erase(0, 2);
        } else if (text[1] == 'b' || text[1] == 'B') {
            base = 2;
            text.erase(0, 2);
        }
    }

    auto iter = text.begin();
    while (iter != text.end()) {
        if (*iter == '_')   iter = text.erase(iter);
        else                ++iter;
    }

    char *endPtr = nullptr;
    long long longResult = std::strtoll(text.c_str(), &endPtr, base);
    if (*endPtr != 0) return IntParseError::INVALID;
    if (base == 10) {
        if (longResult < INT32_MIN) return IntParseError::OUT_OF_RANGE;
        if (longResult > INT32_MAX) return IntParseError::OUT_OF_RANGE;
    } else {
        if (longResult < 0) return IntParseError::OUT_OF_RANGE;
        if (longResult > UINT32_MAX) return IntParseError::OUT_OF_RANGE;
    }

    result = static_cast<int>(longResult);
    return IntParseError::OK;
}
