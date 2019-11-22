#include <sstream>
#include <string>

#include "gameerror.h"
#include "stack.h"


void gtStack::setArgs(const std::vector<Value> &rawArgs, int argCount, int localCount) {
    argList = rawArgs;

    if (static_cast<int>(argList.size()) != argCount) {
        argList.resize(argCount);
    }
    for (int i = 0; i < localCount; ++i) argList.push_back(Value());
}

void gtStack::setArg(unsigned index, const Value &newValue) {
    if (index >= argList.size()) {
        throw GameError("Tried to set illegal local number " + std::to_string(index) + ".");
    }
    argList[index] = newValue;
}

Value gtStack::peek(int index) const {
    if (index < 0) throw GameError("Tried to peek at negative stack index.");
    if (index >= static_cast<int>(mValues.size())) {
        throw GameError("Tried to peek beyond stack size.");
    }
    return mValues[mValues.size() - 1 - index];
}

Value gtStack::popRaw() {
    if (mValues.empty()) {
        throw GameError("Stack underflow.");
    }
    Value value = mValues.back();
    mValues.pop_back();
    return value;
}
Value gtStack::pop() {
    Value value = popRaw();
    if (value.type == Value::LocalVar) {
        if (value.value < 0 || value.value >= static_cast<int>(argList.size())) {
            throw GameError("Illegal argument number.");
        }
        return argList[value.value];
    }
    return value;
}

Value& gtStack::operator[](int index) {
    if (index < 0 || index >= static_cast<int>(mValues.size())) {
        throw GameError("Tried to access invalid stack position.");
    }
    return mValues[index];
}

const Value& gtStack::operator[](int index) const {
    if (index < 0 || index >= static_cast<int>(mValues.size())) {
        throw GameError("Tried to access invalid stack position.");
    }
    return mValues[index];
}


void gtCallStack::create(const FunctionDef &funcDef, unsigned functionId) {
    mFrames.push_back(Frame{funcDef, functionId});
}

void gtCallStack::drop() {
    mFrames.pop_back();
}

gtStack& gtCallStack::getStack() {
    if (mFrames.empty()) {
        throw GameError("Tried to access stack with empty call stack.");
    }
    return mFrames[mFrames.size() - 1].stack;
}


bool gtCallStack::isEmpty() const {
    return mFrames.size() == 0;
}

int gtCallStack::size() const {
    return mFrames.size();
}

const gtCallStack::Frame& gtCallStack::operator[](int index) {
    if (index < 0 || index >= static_cast<int>(mFrames.size())) {
        throw GameError("Tried to read non-exstant stack frame.");
    }
    return mFrames[index];
}
