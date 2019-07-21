#ifndef STACK_H_735463546
#define STACK_H_735463546

#include <vector>
#include "value.h"

class gtStack {
public:
    void setArgs(const std::vector<Value> &rawArgs, int argCount, int localCount);

    Value peek(int index = 0) const;
    void push(const Value &value) {
        mValues.push_back(value);
    }
    Value popRaw();
    Value pop();
    bool isEmpty() const {
        return mValues.empty();
    }
    unsigned size() const {
        return mValues.size();
    }

    Value& operator[](int index);
    const Value& operator[](int index) const;

    std::vector<Value> argList;
    std::vector<Value> mValues;
};

class gtCallStack {
public:
    struct Frame {
        unsigned functionId;
        gtStack stack;
    };

    void create(unsigned functionId);
    void drop();
    gtStack& getStack();

    bool isEmpty() const;
    int size() const;
    const Frame& operator[](int index);
private:
    std::vector<Frame> mFrames;
};

#endif
