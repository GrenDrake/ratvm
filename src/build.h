#ifndef BUILD_H
#define BUILD_H

#include <string>
#include <vector>

class Token;
class GameData;

std::vector<Token> lex_file(GameData &gamedata, const std::string &filename);
std::vector<Token> lex_string(GameData &gamedata, const std::string &source_name, const std::string &text);
void preprocess_tokens(GameData &gamedata, std::vector<Token> &tokens);
int parse_tokens(GameData &gamedata, const std::vector<Token> &tokens);
void translate_symbols(GameData &gamedata);
int parse_functions(GameData &gamedata);
void generate(GameData &gamedata, const std::string &outputFile);

int parseAsInt(const std::string &text);
std::string readFile(const std::string &file);

#endif
