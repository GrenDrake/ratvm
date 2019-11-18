#include <sstream>
#include <unicode/normalizer2.h>
#include "builderror.h"

void normalize(std::string &s) {
    UErrorCode err = U_ZERO_ERROR;
    const icu::Normalizer2 *n = icu::Normalizer2::getInstance(nullptr, "nfc", UNORM2_COMPOSE, err);
    if (U_FAILURE(err)) {
        std::stringstream ss;
        ss << "Failed to get Normalizer2 instance: ";
        ss << u_errorName(err);
        ss << ".";
        throw BuildError(ss.str());
    }

    icu::UnicodeString text = icu::UnicodeString::fromUTF8(s);
    icu::UnicodeString result = n->normalize(text, err);
    if (U_FAILURE(err)) {
        std::stringstream ss;
        ss << "Failed to normalize string text: ";
        ss << u_errorName(err);
        ss << ".";
        throw BuildError(ss.str());
    }
    s.clear();
    result.toUTF8String(s);
}