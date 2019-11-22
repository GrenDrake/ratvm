#ifndef STACK_H_735463546
#define STACK_H_735463546

#include <string>
#include <vector>
#include "value.h"

struct FunctionDef;

class gtStack {
public:
    void setArgs(const std::vector<Value> &rawArgs, int argCount, int localCount);
    void setArg(unsigned index, const Value &newValue);
    int argCount() const {
        return static_cast<int>(argList.size());
    }

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
        return static_cast<unsigned>(mValues.size());
    }

    Value& operator[](int index);
    const Value& operator[](int index) const;

    std::vector<Value> argList;
    std::vector<Value> mValues;
};

class gtCallStack {
public:
    struct Frame {
        const FunctionDef &funcDef;
        unsigned functionId;
        gtStack stack;
        int IP;
    };

    Value peek(int index = 0) const {
        return mFrames.back().stack.peek(index);
    }
    void push(const Value &value) {
        mFrames.back().stack.push(value);
    }
    Value popRaw() {
        return mFrames.back().stack.popRaw();
    }
    Value pop() {
        return mFrames.back().stack.pop();
    }

    const Frame& callTop() const {
        return mFrames.back();
    }
    Frame& callTop() {
        return mFrames.back();
    }
    void setIP(unsigned IP) {
        callTop().IP = IP;
    }
    unsigned getIP() const {
        return callTop().IP;
    }

    void create(const FunctionDef &funcDef, unsigned functionId);
    void drop();
    gtStack& getStack();

    bool isEmpty() const;
    int size() const;
    const Frame& operator[](int index);
private:
    std::vector<Frame> mFrames;
};

#endif
