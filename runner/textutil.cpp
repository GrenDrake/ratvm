#include <sstream>
#include <utf8proc.h>
#include "gameerror.h"
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

std::string codepointToString(int cp) {
    unsigned char dest[6] = { 0 };
    utf8proc_ssize_t length2 = utf8proc_encode_char(cp, dest);
    dest[length2] = 0;
    std::string result = reinterpret_cast<const char*>(dest);
    return result;
}