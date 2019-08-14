#include <iostream>
#include <cctype>
#include <regex>
#include <string>

std::string formatText(const std::string &text) {
    std::regex cWhitespace("[ \t]+");
    std::regex cLinebreak("[\n\r]+");
    std::regex cNewline("\n");
    std::regex cBR("\\[br\\]");
    std::regex cHR("---+");
    std::regex cBold("\\[b](.*?)\\[\\/b]");
    std::regex cItalic("\\[i](.*?)\\[\\/i]");

    std::string result = text;
    result = regex_replace(result, cWhitespace, " ");
    result = regex_replace(result, cLinebreak, "\n");
    result = regex_replace(result, cNewline, "\n\n");
    result = regex_replace(result, cBR, "\n");
    result = regex_replace(result, cHR, "------------------------------------------------------------------------------");
    result = regex_replace(result, cBold, "**$1**");
    result = regex_replace(result, cItalic, "__$1__");

    result.erase(0, result.find_first_not_of(" \n"));
    result.erase(result.find_last_not_of(" \n") + 1);
    return result;
}