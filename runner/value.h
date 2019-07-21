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
        JumpTarget  = 9,
        VarRef      = 10,
        LocalVar    = 15,
    };

    Value()
    : type(None), selfObj(0)
    { }
    Value(Type type, int value)
    : type(type), value(value), selfObj(0)
    { }

    Type type;
    int value;
    unsigned selfObj;

    void requireType(Value::Type theType);
    void requireType(Value::Type typeOne, Value::Type typeTwo);
    bool isTrue() const;
    int compare(const Value &rhs) const;
};

std::ostream& operator<<(std::ostream &out, const Value::Type &type);
std::ostream& operator<<(std::ostream &out, const Value &value);

#endif
