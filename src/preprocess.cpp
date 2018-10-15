/* **************************************************************************
 * Handle preprocessing of tokens
 *
 * run through all tokens and preprocess them to add neccesary metadata as
 * well as handling all include statements and tokens included as a part of
 * them.
 *
 * Part of GTRPE by Gren Drake
 * **************************************************************************/

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "gamedata.h"
#include "build.h"
#include "token.h"


void preprocess_tokens(GameData &gamedata, std::vector<Token> &tokens) {
    std::vector<Token>::iterator iter = tokens.begin();
    while (iter != tokens.end()) {
        if (iter->type == Token::Identifier && iter->text == "include") {
            if ((iter + 1)->type != Token::String) {
                std::stringstream ss;
                ss << "Expected string, but found " << (iter+1)->type << ".";
                gamedata.errors.push_back(Error{(iter+1)->origin, ss.str()});
                return;
            }
            if ((iter + 2)->type != Token::Semicolon) {
                std::stringstream ss;
                ss << "Expected Semicolon, but found " << (iter+2)->type << ".";
                gamedata.errors.push_back(Error{(iter+2)->origin, ss.str()});
                return;
            }
            const std::string &filename = (iter+1)->text;
            std::vector<Token> newtokens = lex_file(gamedata, filename);
            iter = tokens.erase(iter, iter + 3);
            iter = tokens.insert(iter, newtokens.begin(), newtokens.end());
        } else {
            ++iter;
        }
    }
}
