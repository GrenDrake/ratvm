#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <vector>

#include "value.h"

struct List;
struct ListValue {
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

void dump_list(const List *list, std::ostream &out);

#endif
