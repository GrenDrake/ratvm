#include <string>
#include <sstream>

#include "builderror.h"
#include "parsestate.h"

bool ParseState::at_end() const {
    return current_token == tokens.end();
}

bool ParseState::eof() const {
    return at_end() || matches(Token::EndOfFile);
}

const Token* ParseState::here() const {
    if (at_end()) {
        return nullptr;
    }
    return &(*current_token);
}

const Token* ParseState::next() {
    if (at_end()) {
        return nullptr;
    }
    std::advance(current_token, 1);
    return here();
}

const Token* ParseState::peek() const {
    if (current_token == tokens.end() || current_token + 1 == tokens.end()) {
        return nullptr;
    }
    return &(*(current_token+1));
}

bool ParseState::matches(Token::Type type) const {
    const Token *token = here();
    if (!token) return false;
    return token->type == type;
}

bool ParseState::matches(const std::string &text) const {
    const Token *token = here();
    if (!token) return false;
    if (token->type != Token::Identifier) return false;
    return token->text == text;
}

void ParseState::require(Token::Type type) const {
    if (!matches(type)) {
        const Token *token = here();
        if (token) {
            std::stringstream ss;
            ss << "Expected " << type << ", but found ";
            ss << token->type << ".";
            throw BuildError(token->origin, ss.str());
        } else {
            throw BuildError("Unexpected end of tokens.");
        }
    }
}

void ParseState::skip(Token::Type type) {
    require(type);
    next();
}

void ParseState::skip(const std::string &text) {
    require(Token::Identifier);
    if (here()->text == text) {
        next();
        return;
    }
    std::stringstream ss;
    ss << " Expected identifier ~" << text << "~, but found ~";
    ss << here()->text << "~.";
    throw BuildError(here()->origin, ss.str());
}

void ParseState::skipTo(Token::Type type) {
    while (!eof() && here()->type != type) {
        next();
    }
}
