/* **************************************************************************
 * General Class Definitions
 *
 * This file contains definitions and close utilities for several small utility
 * classes used by the compiler.
 *
 * Part of GTRPE by Gren Drake
 * **************************************************************************/
#include <algorithm>
#include <cstring>
#include <fstream>
#include <string>
#include <sstream>

#include "gamedata.h"
#include "symboltable.h"
#include "builderror.h"
#include "origin.h"
#include "token.h"

/* ************************************************************************** *
 * Definitions for BuildError class                                           *
 * ************************************************************************** */
BuildError::BuildError(const std::string &msg)
: std::runtime_error(msg), mRawOrigin(), mRawMessage(msg)
{ }
BuildError::BuildError(const Origin &origin, const std::string &msg)
: std::runtime_error(msg), mRawOrigin(origin), mRawMessage(msg)
{ }
const char* BuildError::what() const throw() {
    std::stringstream ss;
    ss << mRawOrigin << ' ' << mRawMessage << " (unchecked)";
    return ss.str().c_str();
}
const std::string& BuildError::getMessage() const {
    return mRawMessage;
}
const Origin& BuildError::getOrigin() const {
    return mRawOrigin;
}



/* ************************************************************************** *
 * Definitions for Token class                                                *
 * ************************************************************************** */
std::ostream& operator<<(std::ostream &out, const Token::Type &type) {
    switch(type) {
        case Token::Identifier:
            out << "Identifier";
            break;
        case Token::String:
            out << "String";
            break;
        case Token::Integer:
            out << "Integer";
            break;
        case Token::Property:
            out << "Property";
            break;

        case Token::OpenBrace:
            out << "Open Brace";
            break;
        case Token::CloseBrace:
            out << "Close Brace";
            break;
        case Token::OpenParan:
            out << "Open Paran";
            break;
        case Token::CloseParan:
            out << "Close Paran";
            break;
        case Token::OpenSquare:
            out << "Open Square";
            break;
        case Token::CloseSquare:
            out << "Close Square";
            break;

        case Token::Indirection:
            out << "Indirection";
            break;
        case Token::Semicolon:
            out << "Semicolon";
            break;
        case Token::Colon:
            out << "Colon";
            break;
        case Token::EndOfFile:
            out << "End-Of-File";
            break;
        default:
            out << "(unhandled type)";
    }
    return out;
}

std::ostream& operator<<(std::ostream &out, const Token &token) {
    out << '[';
    out << token.origin;
    out << ' ';
    out << token.type;
    switch(token.type) {
        case Token::Integer:
            out << ' ' << token.value;
            break;
        case Token::String:
            out << " ~" << token.text << '~';
            break;
        case Token::Identifier:
            out << ' ' << token.text;
            break;
        case Token::Property:
            out << ' ' << token.text << '[' << token.value << ']';
            break;

        case Token::OpenBrace:
        case Token::CloseBrace:
        case Token::OpenParan:
        case Token::CloseParan:
        case Token::OpenSquare:
        case Token::CloseSquare:
        case Token::Semicolon:
        case Token::Colon:
        case Token::EndOfFile:
            // do nothing
            break;

        default:
            out << " (unhandled type)";
    }
    out << ']';
    return out;
}


/* ************************************************************************** *
 * Definitions for Origin class                                                *
 * ************************************************************************** */
std::ostream& operator<<(std::ostream &out, const Origin &origin) {
    std::stringstream ss;
    ss << origin.file;
    if (origin.line > 0) {
        ss << ':' << origin.line;
        if (origin.column > 0) {
            ss << ':' << origin.column;
        }
    }
    out << ss.str();
    return out;
}


/* ************************************************************************** *
 * Definitions for GameObject class                                             *
 * ************************************************************************** */
void GameObject::addProperty(const Origin &origin, unsigned id, const Value &value) {
    if (getProperty(id) != nullptr) {
        std::stringstream ss;
        ss << "Duplicate property on object.";
        throw BuildError(origin, ss.str());
    }
    properties.push_back(GameProperty{
        origin,
        id,
        value
        });
}

const GameProperty* GameObject::getProperty(unsigned id) const {
    for (const GameProperty &prop : properties) {
        if (prop.id == id) {
            return &prop;
        }
    }
    return nullptr;
}


/* ************************************************************************** *
 * SymbolTable functions                                                      *
 * ************************************************************************** */
void SymbolTable::add(const SymbolDef &symbol) {
    const SymbolDef *oldSymbol = get(symbol.name);
    if (oldSymbol != nullptr) {
        std::stringstream ss;
        ss << "Symbol ~" << symbol.name << "~ already defined at ";
        ss << oldSymbol->origin << '.';
        throw BuildError(symbol.origin, ss.str());
    }
    symbols.push_back(symbol);
}
const SymbolDef* SymbolTable::get(const std::string &name) {
    for (const SymbolDef &symbol : symbols) {
        if (symbol.name == name) {
            return &symbol;
        }
    }
    return nullptr;
}

std::ostream& operator<<(std::ostream &out, const SymbolDef::Type &type) {
    switch(type) {
        case SymbolDef::Object:
            out << "Object";
            break;
        case SymbolDef::Integer:
            out << "Integer";
            break;
        case SymbolDef::String:
            out << "String";
            break;
        default:
            out << "(unhandled type)";
    }
    return out;
}

/* ************************************************************************** *
 * General utility functions                                                  *
 * ************************************************************************** */
std::string readFile(const std::string &file) {
    std::ifstream inf(file);
    if (!inf) {
        throw BuildError(Origin(), "Could not open file "+file+".");
    }
    std::string content( (std::istreambuf_iterator<char>(inf)),
                         std::istreambuf_iterator<char>() );
    return content;
}

int parseAsInt(const std::string &text) {
    int value = 0;
    bool negative = false;
    size_t start = 0;
    if (!text.empty() && text[0] == '-') {
        negative = true;
        start = 1;
    }
    for (size_t pos = start; pos < text.size(); ++pos) {
        char c = text[pos];
        if (c >= '0' && c <= '9') {
            value *= 10;
            value += c - '0';
        } else {
            throw BuildError("invalid integer");
        }
    }
    if (negative) value = -value;
    return value;
}
