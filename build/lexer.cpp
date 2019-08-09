#include <cctype>
#include <cstdint>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "gamedata.h"
#include "builderror.h"
#include "build.h"
#include "origin.h"
#include "token.h"

struct LexerState {
    Origin origin;
    const std::string &text;
    size_t pos;
};

void handle_string_escapes(GameData &gamedata, const Origin &origin, std::string &text);

std::vector<Token> lex_file(GameData &gamedata, const std::string &filename) {
    std::cerr << "[including file ~" << filename << "~.]\n";
    std::string text = readFile(filename);
    return lex_string(gamedata, filename, text);
}


int here(const LexerState &state) {
    if (state.pos >= state.text.size()) {
        return 0;
    }
    return state.text[state.pos];
}

int prev(const LexerState &state) {
    if (state.pos == 0) {
        return 0;
    }
    return state.text[state.pos - 1];
}

int peek(const LexerState &state) {
    if (state.pos + 1 >= state.text.size()) {
        return 0;
    }
    return state.text[state.pos + 1];
}

int next(LexerState &state) {
    if (state.pos >= state.text.size()) {
        return 0;
    }
    if (here(state) == '\n') {
        ++state.origin.line;
        state.origin.column = 1;
    } else {
        ++state.origin.column;
    }
    ++state.pos;
    return here(state);
}

bool isValidIdentifier(int c) {
    if (c == 0) return false;
    if (isalnum(c)) return true;
    if (c == '_') {
        return true;
    }
    return false;
}

void handle_string_escapes(GameData &gamedata, const Origin &origin, std::string &text) {
    size_t spacesStart = SIZE_MAX;
    for (unsigned i = 0; i < text.size(); ++i) {
        if (isspace(text[i]) && spacesStart == SIZE_MAX)  spacesStart = i;
        else if (!isspace(text[i]))                       spacesStart = SIZE_MAX;
        if (text[i] == '\n') {
            size_t whitespaceEnd = i;
            while (whitespaceEnd < text.size() && isspace(text[whitespaceEnd])) {
                ++whitespaceEnd;
            }
            if (spacesStart != 0 && whitespaceEnd != text.size()) {
                text[spacesStart] = ' ';
                text.erase(spacesStart + 1, whitespaceEnd - spacesStart - 1);
            } else {
                text.erase(spacesStart, whitespaceEnd - spacesStart);
            }
            --i;
            continue;
        }

        if (text[i] != '\\') continue;
        switch(text[i + 1]) {
            case '\'':
            case '"':
            case '\\':
                text.erase(i, 1);
                break;
            case 'n':
                text.erase(i, 1);
                text[i] = '\n';
                break;
            default: {
                std::stringstream ss;
                ss << "Unknown string escape \\" << text[i + 1] << '.';
                gamedata.addError(origin, ErrorMsg::Error, ss.str());
            }
        }
    }
}

std::vector<Token> lex_string(GameData &gamedata, const std::string &source_name, const std::string &text) {
    LexerState state = { { source_name, 1, 1}, text };
    std::vector<Token> tokens;


    while (here(state)) {
        int c = here(state);

        if (isspace(c)) {
            while (isspace(c)) {
                c = next(state);
            }
            continue;
        }

        if (c == '/' && peek(state) == '/') {
            while (here(state) != '\n' && here(state) != 0) {
                next(state);
            }
            continue;
        }
        if (c == '/' && peek(state) == '*') {
            next(state);
            next(state);
            while (here(state) != '*' || peek(state) != '/') {
                if (here(state) == '/' && peek(state) == '*') {
                    gamedata.addError(state.origin, ErrorMsg::Error, "Block comments may not be nested.");
                }
                if (here(state) == 0) {
                    gamedata.addError(state.origin, ErrorMsg::Error, "End-of-file in block comment.");
                }
                next(state);
            }
            next(state);
            next(state);
            continue;
        }


        if (c == ';') {
            tokens.push_back(Token(state.origin, Token::Semicolon));
            next(state);
            continue;
        } else if (c == ':') {
            tokens.push_back(Token(state.origin, Token::Colon));
            next(state);
            continue;
        } else if (c == '*') {
            tokens.push_back(Token(state.origin, Token::Indirection));
            next(state);
            continue;
        } else if (c == '[') {
            tokens.push_back(Token(state.origin, Token::OpenSquare));
            next(state);
            continue;
        } else if (c == ']') {
            tokens.push_back(Token(state.origin, Token::CloseSquare));
            next(state);
            continue;
        } else if (c == '{') {
            tokens.push_back(Token(state.origin, Token::OpenBrace));
            next(state);
            continue;
        } else if (c == '}') {
            tokens.push_back(Token(state.origin, Token::CloseBrace));
            next(state);
            continue;
        } else if (c == '(') {
            tokens.push_back(Token(state.origin, Token::OpenParan));
            next(state);
            continue;
        } else if (c == ')') {
            tokens.push_back(Token(state.origin, Token::CloseParan));
            next(state);
            continue;

        } else if (isdigit(c) || c == '-') {
            Origin origin = state.origin;
            bool isNegative = false;
            if (c == '-') {
                isNegative = true;
                next(state);
            }
            size_t start = state.pos;
            while (isdigit(here(state))) {
                next(state);
            }
            if (state.pos == start) {
                gamedata.addError(state.origin, ErrorMsg::Error, "Numbers must consist of at least one digit.");
            }
            std::string text = state.text.substr(start, state.pos - start);
            int value = parseAsInt(text);
            if (isNegative) value = -value;
            tokens.push_back(Token(origin, Token::Integer, value));
            continue;

        } else if (c == '"' || c == '\'') {
            Origin origin = state.origin;
            int quote_char = c;
            next(state);
            size_t start = state.pos;
            while (here(state) && here(state) != quote_char) {
                if (here(state) == '\\') next(state);
                next(state);
            }
            std::string text = state.text.substr(start, state.pos - start);
            handle_string_escapes(gamedata, origin, text);
            if (quote_char == '"') {
                if (text.size() > UINT16_MAX) {
                    std::stringstream ss;
                    ss << "WARNING String exceeds max string length of ";
                    ss << (UINT16_MAX - 1) << "; excess data truncated.";
                    gamedata.addError(origin, ErrorMsg::Error, ss.str());
                    text.erase(UINT16_MAX);
                }
                tokens.push_back(Token(origin, Token::String, text));
            } else {
                if (text.size() != 1) {
                    gamedata.addError(origin, ErrorMsg::Error,
                        "character literal has invalid length");
                } else {
                    tokens.push_back(Token(origin, Token::Integer, text[0]));
                }
            }
            next(state);
            continue;

        } else if (c == '$') {
            Origin origin = state.origin;
            next(state);
            size_t start = state.pos;
            while (isValidIdentifier(here(state))) {
                next(state);
            }
            std::string text = state.text.substr(start, state.pos - start);
            if (text.empty()) {
                    gamedata.addError(origin, ErrorMsg::Error,
                        "found empty property name");
            }
            unsigned propertyId = gamedata.getPropertyId(text);
            tokens.push_back(Token(origin, Token::Property, text, propertyId));
            continue;

        } else if (isValidIdentifier(c)) {
            Origin origin = state.origin;
            size_t start = state.pos;
            while (isValidIdentifier(here(state))) {
                next(state);
            }
            std::string text = state.text.substr(start, state.pos - start);
            tokens.push_back(Token(origin, Token::Identifier, text));
            continue;

        } else {
            std::stringstream ss;
            ss << "Unexpected '" << (char)c << "' encountered.";
            gamedata.addError(state.origin, ErrorMsg::Error, ss.str());
            next(state);
        }

    }

    tokens.push_back(Token(state.origin, Token::EndOfFile));
    return tokens;
}
