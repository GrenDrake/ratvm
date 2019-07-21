#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include "gamedata.h"
#include "opcode.h"

#include "stack.h"

const int MAX_RUNTIME = 1000000000;

Value GameData::runFunctionCore(unsigned functionId, std::vector<Value> rawArgList) {
    int iterations = 0;

    const FunctionDef &funcDef = functions[functionId];
    callStack.create(functionId);
    callStack.getStack().setArgs(rawArgList, funcDef.arg_count, funcDef.local_count);
    std::vector<Value> &argList = callStack.getStack().argList;

    unsigned baseAddress = functions[functionId].position;
    unsigned operationCount = 0;
    unsigned IP = baseAddress;

    while (1) {
        if (iterations > MAX_RUNTIME) {
            std::stringstream ss;
            ss << "Function exceeded max runtime at IP:";
            ss << IP << " (local offset: " << IP - baseAddress << ").";
            throw GameError(ss.str());
        }
        ++iterations;

        int opcode = bytecode.read_8(IP);
        ++IP;
        ++operationCount;

        switch(opcode) {
            case OpcodeDef::Return: {
                Value retValue = noneValue;
                if (!callStack.getStack().isEmpty()) {
                    retValue = callStack.getStack().pop();
                }
                callStack.drop();
                return retValue; }

            case OpcodeDef::Push0: {
                int type = bytecode.read_8(IP);
                ++IP;
                callStack.getStack().push(Value(static_cast<Value::Type>(type), 0));
                break; }
            case OpcodeDef::Push1: {
                int type = bytecode.read_8(IP);
                ++IP;
                callStack.getStack().push(Value(static_cast<Value::Type>(type), 1));
                break; }
            case OpcodeDef::PushNone: {
                callStack.getStack().push(noneValue);
                break; }
            case OpcodeDef::Push8: {
                int type = bytecode.read_8(IP);
                ++IP;
                int value = bytecode.read_8(IP);
                ++IP;
                if (value & 0x80) value |= 0xFFFFFF00;
                callStack.getStack().push(Value(static_cast<Value::Type>(type), value));
                break; }
            case OpcodeDef::Push16: {
                int type = bytecode.read_8(IP);
                ++IP;
                int value = bytecode.read_16(IP);
                IP += 2;
                if (value & 0x8000) value |= 0xFFFF0000;
                callStack.getStack().push(Value(static_cast<Value::Type>(type), value));
                break; }
            case OpcodeDef::Push32: {
                int type = bytecode.read_8(IP);
                ++IP;
                int value = bytecode.read_32(IP);
                IP += 4;
                callStack.getStack().push(Value(static_cast<Value::Type>(type), value));
                break; }
            case OpcodeDef::Store: {
                Value localId = callStack.getStack().popRaw();
                Value value = callStack.getStack().pop();
                localId.requireType(Value::LocalVar);
                if (localId.value < 0 || localId.value >= static_cast<int>(argList.size())) {
                    throw GameError("Illegal local number.");
                }
                argList[localId.value] = value;
                break; }

            case OpcodeDef::SayUCFirst: {
                Value theText = callStack.getStack().pop();
                if (theText.type == Value::String) {
                    std::string toSay = strings[theText.value].text;
                    if (!toSay.empty()) {
                        toSay[0] = std::toupper(toSay[0]);
                        say(toSay);
                    }
                } else say(theText);
                break; }
            case OpcodeDef::Say: {
                Value theText = callStack.getStack().pop();
                say(theText);
                break; }
            case OpcodeDef::SayUnsigned: {
                Value theNumber = callStack.getStack().pop();
                theNumber.requireType(Value::Integer);
                say(std::to_string(static_cast<unsigned>(theNumber.value)));
                break; }
            case OpcodeDef::SayChar: {
                Value theText = callStack.getStack().pop();
                theText.requireType(Value::Integer);
                std::string aString(" ");
                aString[0] = theText.value;
                say(aString);
                break; }

            case OpcodeDef::StackPop: {
                callStack.getStack().pop();
                break; }
            case OpcodeDef::StackDup: {
                callStack.getStack().push(callStack.getStack().peek());
                break; }
            case OpcodeDef::StackPeek: {
                Value index = callStack.getStack().pop();
                index.requireType(Value::Integer);
                callStack.getStack().push(callStack.getStack().peek(index.value));
                break; }
            case OpcodeDef::StackSize: {
                callStack.getStack().push(Value(Value::Integer, callStack.getStack().size()));
                break; }

            case OpcodeDef::Call: {
                Value functionId = callStack.getStack().pop();
                Value argCount = callStack.getStack().pop();
                functionId.requireType(Value::Function);
                argCount.requireType(Value::Integer);
                std::vector<Value> funcArgs;
                if (functionId.selfObj > 0) {
                    funcArgs.push_back(Value(Value::Object, functionId.selfObj));
                } else {
                    funcArgs.push_back(noneValue);
                }
                for (int i = 0; i < argCount.value; ++i) {
                    funcArgs.push_back(callStack.getStack().pop());
                }
                Value result = runFunctionCore(functionId.value, funcArgs);
                callStack.getStack().push(result);
                break; }

            case OpcodeDef::GetItem: {
                Value from = callStack.getStack().pop();
                Value index = callStack.getStack().pop();
                Value result;
                switch(from.type) {
                    case Value::Object:
                        index.requireType(Value::Property);
                        result = objects[from.value].get(index.value);
                        break;
                    default:
                        throw GameError("get requires list, map, or object.");
                }
                callStack.getStack().push(result);
                break;
            }
            case OpcodeDef::HasItem: {
                Value from = callStack.getStack().pop();
                Value index = callStack.getStack().pop();
                bool result;
                switch(from.type) {
                    case Value::Object:
                        index.requireType(Value::Property);
                        result = objects[from.value].has(index.value);
                        break;
                    default:
                        throw GameError("has requires list, map, or object.");
                }
                callStack.getStack().push(Value{Value::Integer, result ? 1 : 0});
                break;
            }
            case OpcodeDef::GetSize: {
                Value list = callStack.getStack().pop();
                list.requireType(Value::List);
                const ListDef &def = getList(list.value);
                callStack.getStack().push(Value(Value::Integer, def.items.size()));
                break; }
            case OpcodeDef::SetItem: {
                Value from = callStack.getStack().pop();
                Value index = callStack.getStack().pop();
                Value toValue = callStack.getStack().pop();
                switch(from.type) {
                    case Value::Object:
                        index.requireType(Value::Property);
                        objects[from.value].set(index.value, toValue);
                        break;
                    default:
                        throw GameError("setp requires list, map, or object.");
                }

                break; }
            case OpcodeDef::TypeOf: {
                Value ofWhat = callStack.getStack().pop();
                callStack.getStack().push(Value{Value::Integer, static_cast<int>(ofWhat.type)});
                break; }
            case OpcodeDef::AsType: {
                Value ofWhat = callStack.getStack().pop();
                Value toType = callStack.getStack().pop();
                toType.requireType(Value::Integer);
                callStack.getStack().push(Value{static_cast<Value::Type>(toType.value), ofWhat.value});
                break; }

            case OpcodeDef::Equal: {
                Value rhs = callStack.getStack().pop();
                Value lhs = callStack.getStack().pop();
                callStack.getStack().push(Value{Value::Integer, !lhs.compare(rhs)});
                break; }
            case OpcodeDef::NotEqual: {
                Value rhs = callStack.getStack().pop();
                Value lhs = callStack.getStack().pop();
                callStack.getStack().push(Value{Value::Integer, lhs.compare(rhs)});
                break; }


            case OpcodeDef::Jump: {
                Value target = callStack.getStack().pop();
                target.requireType(Value::JumpTarget);
                IP = baseAddress + target.value;
                break; }
            case OpcodeDef::JumpZero: {
                Value target = callStack.getStack().pop();
                Value condition = callStack.getStack().pop();
                target.requireType(Value::JumpTarget);
                if (!condition.isTrue()) {
                    IP = baseAddress + target.value;
                }
                break; }
            case OpcodeDef::JumpNotZero: {
                Value target = callStack.getStack().pop();
                Value condition = callStack.getStack().pop();
                target.requireType(Value::JumpTarget);
                if (condition.isTrue()) {
                    IP = baseAddress + target.value;
                }
                break; }
            case OpcodeDef::LessThan: {
                Value rhs = callStack.getStack().pop();
                Value lhs = callStack.getStack().pop();
                callStack.getStack().push(Value{Value::Integer, lhs.compare(rhs) > 0});
                break; }
            case OpcodeDef::LessThanEqual: {
                Value rhs = callStack.getStack().pop();
                Value lhs = callStack.getStack().pop();
                callStack.getStack().push(Value{Value::Integer, lhs.compare(rhs) >= 0});
                break; }
            case OpcodeDef::GreaterThan: {
                Value rhs = callStack.getStack().pop();
                Value lhs = callStack.getStack().pop();
                callStack.getStack().push(Value{Value::Integer, lhs.compare(rhs) < 0});
                break; }
            case OpcodeDef::GreaterThanEqual: {
                Value rhs = callStack.getStack().pop();
                Value lhs = callStack.getStack().pop();
                callStack.getStack().push(Value{Value::Integer, lhs.compare(rhs) <= 0});
                break; }

            case OpcodeDef::Not: {
                Value v = callStack.getStack().pop();
                if (v.isTrue()) callStack.getStack().push(Value(Value::Integer, 0));
                else            callStack.getStack().push(Value(Value::Integer, 1));
                break; }
            case OpcodeDef::Add: {
                Value rhs = callStack.getStack().pop();
                Value lhs = callStack.getStack().pop();
                lhs.requireType(Value::Integer);
                rhs.requireType(Value::Integer);
                callStack.getStack().push(Value{Value::Integer, rhs.value + lhs.value});
                break; }
            case OpcodeDef::Sub: {
                Value rhs = callStack.getStack().pop();
                Value lhs = callStack.getStack().pop();
                lhs.requireType(Value::Integer);
                rhs.requireType(Value::Integer);
                callStack.getStack().push(Value{Value::Integer, rhs.value - lhs.value});
                break; }
            case OpcodeDef::Mult: {
                Value rhs = callStack.getStack().pop();
                Value lhs = callStack.getStack().pop();
                lhs.requireType(Value::Integer);
                rhs.requireType(Value::Integer);
                callStack.getStack().push(Value{Value::Integer, rhs.value * lhs.value});
                break; }
            case OpcodeDef::Div: {
                Value rhs = callStack.getStack().pop();
                Value lhs = callStack.getStack().pop();
                lhs.requireType(Value::Integer);
                rhs.requireType(Value::Integer);
                callStack.getStack().push(Value{Value::Integer, rhs.value / lhs.value});
                break; }
            case OpcodeDef::Mod: {
                Value rhs = callStack.getStack().pop();
                Value lhs = callStack.getStack().pop();
                lhs.requireType(Value::Integer);
                rhs.requireType(Value::Integer);
                callStack.getStack().push(Value{Value::Integer, rhs.value % lhs.value});
                break; }
            case OpcodeDef::Pow: {
                Value lhs = callStack.getStack().pop();
                Value rhs = callStack.getStack().pop();
                lhs.requireType(Value::Integer);
                rhs.requireType(Value::Integer);
                int result = 1;
                for (int i = 0; i < rhs.value; ++i) result *= lhs.value;
                callStack.getStack().push(Value{Value::Integer, result});
                break; }
            case OpcodeDef::BitLeft: {
                Value v1 = callStack.getStack().pop();
                Value v2 = callStack.getStack().pop();
                v1.requireType(Value::Integer);
                v2.requireType(Value::Integer);
                callStack.getStack().push(Value(Value::Integer, v1.value << v2.value));
                break; }
            case OpcodeDef::BitRight: {
                Value v1 = callStack.getStack().pop();
                Value v2 = callStack.getStack().pop();
                v1.requireType(Value::Integer);
                v2.requireType(Value::Integer);
                callStack.getStack().push(Value(Value::Integer, v1.value >> v2.value));
                break; }
            case OpcodeDef::BitAnd: {
                Value v1 = callStack.getStack().pop();
                Value v2 = callStack.getStack().pop();
                v1.requireType(Value::Integer);
                v2.requireType(Value::Integer);
                callStack.getStack().push(Value(Value::Integer, v1.value & v2.value));
                break; }
            case OpcodeDef::BitOr: {
                Value v1 = callStack.getStack().pop();
                Value v2 = callStack.getStack().pop();
                v1.requireType(Value::Integer);
                v2.requireType(Value::Integer);
                callStack.getStack().push(Value(Value::Integer, v1.value | v2.value));
                break; }
            case OpcodeDef::BitXor: {
                Value v1 = callStack.getStack().pop();
                Value v2 = callStack.getStack().pop();
                v1.requireType(Value::Integer);
                v2.requireType(Value::Integer);
                callStack.getStack().push(Value(Value::Integer, v1.value ^ v2.value));
                break; }
            case OpcodeDef::BitNot: {
                Value v = callStack.getStack().pop();
                v.requireType(Value::Integer);
                callStack.getStack().push(Value(Value::Integer, ~v.value));
                break; }
            case OpcodeDef::Random: {
                Value max = callStack.getStack().pop();
                Value min = callStack.getStack().pop();
                min.requireType(Value::Integer);
                max.requireType(Value::Integer);
                int result = min.value + rand() % (max.value - min.value);
                callStack.getStack().push(Value{Value::Integer, result});
                break; }

            case OpcodeDef::StackSwap: {
                Value idx1 = callStack.getStack().pop();
                Value idx2 = callStack.getStack().pop();
                idx1.requireType(Value::Integer);
                idx2.requireType(Value::Integer);
                Value tmp = callStack.getStack()[idx1.value];
                callStack.getStack()[idx1.value] = callStack.getStack()[idx2.value];
                callStack.getStack()[idx2.value] = tmp;
                break; }

            case OpcodeDef::SetSetting: {
                Value settingNumber = callStack.getStack().pop();
                Value newValue = callStack.getStack().pop();
                break; }

            case OpcodeDef::GetOption: {
                Value functionId = callStack.getStack().pop();
                functionId.requireType(Value::Function);
                optionType = OptionType::Choice;
                optionFunction = functionId.value;
                break; }
            case OpcodeDef::AddOption: {
                Value hotkey = callStack.getStack().pop();
                Value extra = callStack.getStack().pop();
                Value value = callStack.getStack().pop();
                Value text = callStack.getStack().pop();
                text.requireType(Value::String);
                hotkey.requireType(Value::Integer, Value::None);
                options.push_back(GameOption{text.value, value, extra,
                                  hotkey.type == Value::None ? -1 : hotkey.value});
                break; }

            case OpcodeDef::Error: {
                Value msg = callStack.getStack().pop();
                msg.requireType(Value::String);
                throw GameError(strings[msg.value].text);
                break; }
            case OpcodeDef::Origin: {
                Value ofWhat = callStack.getStack().pop();
                std::string text = getSource(ofWhat);
                callStack.getStack().push(Value{Value::String, 0});
                break; }

            default: {
                std::stringstream ss;
                ss << "Unrecognized opcode " << opcode << '.';
                throw GameError(ss.str());
            }
        }
    }
    return noneValue;
}
