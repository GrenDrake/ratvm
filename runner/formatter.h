#ifndef FORMATTER_H
#define FORMATTER_H

#include <string>
#include <vector>

struct ParseResult {
    std::vector<std::string> errors;
    std::string finalResult;

    void addError(const std::string &text) {
        errors.push_back(text);
    }
};

ParseResult formatText(const std::string &text);

#endif
