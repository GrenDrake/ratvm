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

