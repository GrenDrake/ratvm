#include <ostream>
#include "expression.h"

void dump_list(const List *list, std::ostream &out) {
    if (!list) return;
    out << "( ";
    for (const ListValue &v : list->values) {
        if (v.value.type == Value::Expression) {
            dump_list(v.list, out);
        } else {
            out << v.value;
        }
        out << ' ';
    }
    out << ')';
}