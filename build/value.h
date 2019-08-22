#ifndef VALUE_H
#define VALUE_H

#include <string>
struct OpcodeDef;

struct Value {
    enum Type {
        None        = 0,
        Integer     = 1,
        String      = 2,
        List        = 3,
        Map         = 4,
        Function    = 5,
        Object      = 6,
        Property    = 7,
        TypeId      = 8,
        JumpTarget  = 9,
        VarRef      = 10,
        LocalVar    = 15,
        Reserved    = 95,
        Opcode      = 96,
        Expression  = 97,
        FlagSet     = 98,
        Symbol      = 99
    };

    Type type;
    int value;
    std::string text;
    const OpcodeDef *opcode;
};

std::ostream& operator<<(std::ostream &out, const Value::Type &type);
std::ostream& operator<<(std::ostream &out, const Value &value);

#endif
