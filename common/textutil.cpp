#include <sstream>
#include <utf8proc.h>
#include "textutil.h"

bool c_isspace(int c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void normalize(std::string &s) {
    const unsigned char *source = reinterpret_cast<const unsigned char*>(s.c_str());
    char *result = reinterpret_cast<char*>(utf8proc_NFC(source));
    s = result;
    free(result);
}

void upperFirst(std::string &s) {
    const unsigned char *source = reinterpret_cast<const unsigned char*>(s.c_str());
    unsigned char dest[6] = { 0 };
    utf8proc_int32_t codepoint = 0;
    utf8proc_ssize_t length = utf8proc_iterate(source, -1, &codepoint);
    if (codepoint == -1) return;
    s.erase(0, length);
    codepoint = utf8proc_toupper(codepoint);
    utf8proc_ssize_t length2 = utf8proc_encode_char(codepoint, dest);
    dest[length2] = 0;
    s.insert(0, reinterpret_cast<char*>(dest));
}

int getFirstCodepoint(const std::string &s) {
    const unsigned char *source = reinterpret_cast<const unsigned char*>(s.c_str());
    utf8proc_int32_t codepoint = 0;
    utf8proc_iterate(source, -1, &codepoint);
    return codepoint;
}

std::string codepointToString(int cp) {
    unsigned char dest[6] = { 0 };
    utf8proc_ssize_t length2 = utf8proc_encode_char(cp, dest);
    dest[length2] = 0;
    std::string result = reinterpret_cast<const char*>(dest);
    return result;
}

static const char *whitespaceChars = " \n\r\t";
std::string& trim(std::string &text) {
    std::string::size_type pos;

    pos = text.find_first_not_of(whitespaceChars);
    if (pos > 0) text.erase(0, pos);
    pos = text.find_last_not_of(whitespaceChars);
    text.erase(pos + 1);

    return text;
}

std::string trim(const std::string &text) {
    std::string result(text);
    trim(result);
    return result;
}

std::vector<std::string> explodeString(const std::string &s) {
    std::vector<std::string> parts;
    const char *whitespace = " \t\n\r";
    std::string::size_type p = 0, n = 0;

    if (s.empty()) return parts;

    p = s.find_first_not_of(whitespace);

    while (n != std::string::npos) {
        while (p < s.size() && c_isspace(s[p])) ++p;
        if (p >= s.size()) break;
        n = s.find_first_of(whitespace, p);
        std::string sub = s.substr(p, n - p);
        if (!sub.empty()) {
            parts.push_back(sub);
        }
        p = n + 1;
    }

    return parts;
}

bool isValidIdentifier(int c) {
    if (c >= '0' && c <= '9') return true;
    if (c >= 'a' && c <= 'z') return true;
    if (c >= 'A' && c <= 'Z') return true;
    if (c == '_' || c == '-') return true;
    return false;
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
