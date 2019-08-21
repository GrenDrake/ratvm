#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <vector>

#include "value.h"
#include "origin.h"

class GameData;
struct FunctionDef;

struct List;
struct ListValue {
    Origin origin;
    Value value;
    List *list;
};

struct List {
    ~List() {
        for (ListValue &value : values) {
            if (value.value.type == Value::Expression) {
                delete value.list;
            }
        }
    }
    std::vector<ListValue> values;
};

struct StatementType {
    std::string name;
    void (*handler)(GameData &gamedata, FunctionDef *function, List *list);
    bool hasResult;
};

void dump_list(const List *list, std::ostream &out);
bool checkListSize(const List *list, int minSize, int maxSize);
void process_list(GameData &gamedata, FunctionDef *function, List *list);
const StatementType& getReservedWord(const std::string &word);

#endif
