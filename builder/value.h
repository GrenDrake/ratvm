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
        Colon       = 93,
        Indirection = 94,
        Reserved    = 95,
        Opcode      = 96,
        Expression  = 97,
        FlagSet     = 98,
        Symbol      = 99
    };

    Value()
    : type(Integer), value(0), opcode(nullptr)
    { }
    Value(Type type)
    : type(type), value(0), opcode(nullptr)
    { }
    Value(Type type, int value)
    : type(type), value(value), opcode(nullptr)
    { }
    Value(Type type, int value, const std::string &text)
    : type(type), value(value), text(text), opcode(nullptr)
    { }

    Type type;
    int value;
    std::string text;
    const OpcodeDef *opcode;
};

std::ostream& operator<<(std::ostream &out, const Value::Type &type);
std::ostream& operator<<(std::ostream &out, const Value &value);

#endif
