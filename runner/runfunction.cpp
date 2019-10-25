#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include "gamedata.h"
#include "opcode.h"

#include "stack.h"

Value GameData::resume(bool pushValue, const Value &inValue) {
    if (pushValue) callStack.push(inValue);
    unsigned IP = callStack.callTop().IP;

    while (1) {
        ++instructionCount;

        int opcode = bytecode.read_8(IP);
        ++IP;

        switch(opcode) {
            case OpcodeDef::Return: {
                Value retValue = noneValue;
                if (!callStack.getStack().isEmpty()) {
                    retValue = callStack.pop();
                }
                callStack.drop();
                if (callStack.isEmpty()) {
                    optionType = OptionType::EndOfProgram;
                    return retValue;
                } else {
                    callStack.push(retValue);
                    IP = callStack.callTop().IP;
                }
                break; }

            case OpcodeDef::Push0: {
                int type = bytecode.read_8(IP);
                ++IP;
                callStack.push(Value(static_cast<Value::Type>(type), 0));
                break; }
            case OpcodeDef::Push1: {
                int type = bytecode.read_8(IP);
                ++IP;
                callStack.push(Value(static_cast<Value::Type>(type), 1));
                break; }
            case OpcodeDef::PushNone: {
                callStack.push(noneValue);
                break; }
            case OpcodeDef::Push8: {
                int type = bytecode.read_8(IP);
                ++IP;
                int value = bytecode.read_8(IP);
                ++IP;
                if (value & 0x80) value |= 0xFFFFFF00;
                callStack.push(Value(static_cast<Value::Type>(type), value));
                break; }
            case OpcodeDef::Push16: {
                int type = bytecode.read_8(IP);
                ++IP;
                int value = bytecode.read_16(IP);
                IP += 2;
                if (value & 0x8000) value |= 0xFFFF0000;
                callStack.push(Value(static_cast<Value::Type>(type), value));
                break; }
            case OpcodeDef::Push32: {
                int type = bytecode.read_8(IP);
                ++IP;
                int value = bytecode.read_32(IP);
                IP += 4;
                callStack.push(Value(static_cast<Value::Type>(type), value));
                break; }
            case OpcodeDef::Store: {
                Value localId = callStack.popRaw();
                Value value = callStack.pop();
                localId.requireType(Value::LocalVar);
                if (localId.value < 0 || localId.value >=
                        static_cast<int>(callStack.getStack().argCount())) {
                    throw GameError("Illegal local number.");
                }
                callStack.getStack().setArg(localId.value, value);
                break; }

            case OpcodeDef::CollectGarbage: {
                callStack.push(Value(Value::Integer, collectGarbage()));
                break; }

            case OpcodeDef::SayUCFirst: {
                Value theText = callStack.pop();
                if (theText.type == Value::String) {
                    std::string toSay = getString(theText.value).text;
                    if (!toSay.empty()) {
                        toSay[0] = std::toupper(toSay[0]);
                        say(toSay);
                    }
                } else say(theText);
                break; }
            case OpcodeDef::Say: {
                Value theText = callStack.pop();
                say(theText);
                break; }
            case OpcodeDef::SayUnsigned: {
                Value theNumber = callStack.pop();
                theNumber.requireType(Value::Integer);
                say(std::to_string(static_cast<unsigned>(theNumber.value)));
                break; }
            case OpcodeDef::SayChar: {
                Value theText = callStack.pop();
                theText.requireType(Value::Integer);
                std::string aString(" ");
                aString[0] = theText.value;
                say(aString);
                break; }

            case OpcodeDef::StackPop: {
                callStack.pop();
                break; }
            case OpcodeDef::StackDup: {
                callStack.push(callStack.peek());
                break; }
            case OpcodeDef::StackPeek: {
                Value index = callStack.pop();
                index.requireType(Value::Integer);
                callStack.push(callStack.peek(index.value));
                break; }
            case OpcodeDef::StackSize: {
                callStack.push(Value(Value::Integer, callStack.getStack().size()));
                break; }

            case OpcodeDef::Call: {
                Value functionId = callStack.pop();
                Value argCount = callStack.pop();
                functionId.requireType(Value::Function);
                argCount.requireType(Value::Integer);
                std::vector<Value> funcArgs;
                if (functionId.selfObj > 0) {
                    funcArgs.push_back(Value(Value::Object, functionId.selfObj));
                } else {
                    funcArgs.push_back(noneValue);
                }
                for (int i = 0; i < argCount.value; ++i) {
                    funcArgs.push_back(callStack.pop());
                }

                callStack.callTop().IP = IP;
                const FunctionDef &newFunc = functions[functionId.value];
                callStack.create(newFunc, functionId.value);
                callStack.getStack().setArgs(funcArgs,
                        callStack.callTop().funcDef.arg_count,
                        callStack.callTop().funcDef.local_count);
                IP = newFunc.position;
                break; }

            case OpcodeDef::IsValid: {
                Value value = callStack.pop();
                callStack.push(Value(Value::Integer, isValid(value)));
                break; }

            case OpcodeDef::ListPush: {
                Value listId = callStack.pop();
                Value value = callStack.pop();
                listId.requireType(Value::List);
                ListDef &list = getList(listId.value);
                list.items.push_back(value);
                break; }
            case OpcodeDef::ListPop: {
                Value listId = callStack.pop();
                listId.requireType(Value::List);
                ListDef &list = getList(listId.value);
                Value value = list.items.back();
                list.items.pop_back();
                callStack.push(value);
                break; }

            case OpcodeDef::Sort: {
                Value listId = callStack.pop();
                listId.requireType(Value::List);
                sortList(listId);
                break; }
            case OpcodeDef::GetItem: {
                Value from = callStack.pop();
                Value index = callStack.pop();
                Value result;
                switch(from.type) {
                    case Value::Object:
                        index.requireType(Value::Property);
                        result = getObject(from.value).get(*this, index.value);
                        break;
                    case Value::List: {
                        index.requireType(Value::Integer);
                        result = getList(from.value).get(index.value);
                        break; }
                    case Value::Map: {
                        const MapDef &mapDef = getMap(from.value);
                        result = mapDef.get(index);
                        break; }
                    default:
                        throw GameError("get requires list, map, or object.");
                }
                callStack.push(result);
                break;
            }
            case OpcodeDef::HasItem: {
                Value from = callStack.pop();
                Value index = callStack.pop();
                bool result;
                switch(from.type) {
                    case Value::Object:
                        index.requireType(Value::Property);
                        result = getObject(from.value).has(index.value);
                        break;
                    case Value::List: {
                        index.requireType(Value::Integer);
                        const ListDef &listDef = getList(from.value);
                        result = listDef.has(index.value);
                        break; }
                    case Value::Map: {
                        const MapDef &mapDef = getMap(from.value);
                        result = mapDef.has(index);
                        break; }
                    default:
                        throw GameError("has requires list, map, or object.");
                }
                callStack.push(Value{Value::Integer, result ? 1 : 0});
                break;
            }
            case OpcodeDef::GetSize: {
                Value list = callStack.pop();
                list.requireType(Value::List);
                const ListDef &def = getList(list.value);
                callStack.push(Value(Value::Integer, def.items.size()));
                break; }
            case OpcodeDef::SetItem: {
                Value from = callStack.pop();
                Value index = callStack.pop();
                Value toValue = callStack.pop();
                switch(from.type) {
                    case Value::Object:
                        index.requireType(Value::Property);
                        getObject(from.value).set(index.value, toValue);
                        break;
                    case Value::List:
                        index.requireType(Value::Integer);
                        getList(from.value).set(index.value, toValue);
                        break;
                    case Value::Map: {
                        MapDef &mapDef = getMap(from.value);
                        mapDef.set(index, toValue);
                        break; }
                    default:
                        throw GameError("setp requires list, map, or object.");
                }
                break; }
            case OpcodeDef::TypeOf: {
                Value ofWhat = callStack.pop();
                callStack.push(Value{Value::TypeId, static_cast<int>(ofWhat.type)});
                break; }
            case OpcodeDef::DelItem: {
                Value target = callStack.pop();
                Value index = callStack.pop();
                target.requireType(Value::List, Value::Map);
                if (target.type == Value::List) {
                    index.requireType(Value::Integer);
                    ListDef &listDef = getList(target.value);
                    listDef.del(index.value);
                } else if (target.type == Value::Map) {
                    MapDef &mapDef = getMap(target.value);
                    mapDef.del(index);
                } else {
                    throw GameError("not implemented");
                }
                break; }
            case OpcodeDef::InsItem: {
                Value theList = callStack.pop();
                Value theIndex = callStack.pop();
                Value theValue = callStack.pop();
                theList.requireType(Value::List);
                theIndex.requireType(Value::Integer);
                theValue.forbidType(Value::VarRef);
                ListDef &listDef = getList(theList.value);
                if (theIndex.value < 0) theIndex.value = 0;
                if (theIndex.value > static_cast<int>(listDef.items.size())) {
                    theIndex.value = listDef.items.size();
                }
                listDef.items.insert(listDef.items.begin() + theIndex.value,
                                     theValue);
                break; }
            case OpcodeDef::AsType: {
                Value ofWhat = callStack.pop();
                Value toType = callStack.pop();
                toType.requireType(Value::TypeId);
                callStack.push(Value{static_cast<Value::Type>(toType.value), ofWhat.value});
                break; }

            case OpcodeDef::Equal: {
                Value rhs = callStack.pop();
                Value lhs = callStack.pop();
                callStack.push(Value{Value::Integer, !lhs.compare(rhs)});
                break; }
            case OpcodeDef::NotEqual: {
                Value rhs = callStack.pop();
                Value lhs = callStack.pop();
                callStack.push(Value{Value::Integer, lhs.compare(rhs)});
                break; }


            case OpcodeDef::Jump: {
                Value target = callStack.pop();
                target.requireType(Value::JumpTarget);
                IP = callStack.callTop().funcDef.position + target.value;
                break; }
            case OpcodeDef::JumpZero: {
                Value target = callStack.pop();
                Value condition = callStack.pop();
                target.requireType(Value::JumpTarget);
                if (!condition.isTrue()) {
                    IP = callStack.callTop().funcDef.position + target.value;
                }
                break; }
            case OpcodeDef::JumpNotZero: {
                Value target = callStack.pop();
                Value condition = callStack.pop();
                target.requireType(Value::JumpTarget);
                if (condition.isTrue()) {
                    IP = callStack.callTop().funcDef.position + target.value;
                }
                break; }
            case OpcodeDef::LessThan: {
                Value rhs = callStack.pop();
                Value lhs = callStack.pop();
                callStack.push(Value{Value::Integer, lhs.compare(rhs) > 0});
                break; }
            case OpcodeDef::LessThanEqual: {
                Value rhs = callStack.pop();
                Value lhs = callStack.pop();
                callStack.push(Value{Value::Integer, lhs.compare(rhs) >= 0});
                break; }
            case OpcodeDef::GreaterThan: {
                Value rhs = callStack.pop();
                Value lhs = callStack.pop();
                callStack.push(Value{Value::Integer, lhs.compare(rhs) < 0});
                break; }
            case OpcodeDef::GreaterThanEqual: {
                Value rhs = callStack.pop();
                Value lhs = callStack.pop();
                callStack.push(Value{Value::Integer, lhs.compare(rhs) <= 0});
                break; }

            case OpcodeDef::Not: {
                Value v = callStack.pop();
                if (v.isTrue()) callStack.push(Value(Value::Integer, 0));
                else            callStack.push(Value(Value::Integer, 1));
                break; }
            case OpcodeDef::Add: {
                Value rhs = callStack.pop();
                Value lhs = callStack.pop();
                lhs.requireType(Value::Integer);
                rhs.requireType(Value::Integer);
                callStack.push(Value{Value::Integer, rhs.value + lhs.value});
                break; }
            case OpcodeDef::Sub: {
                Value rhs = callStack.pop();
                Value lhs = callStack.pop();
                lhs.requireType(Value::Integer);
                rhs.requireType(Value::Integer);
                callStack.push(Value{Value::Integer, rhs.value - lhs.value});
                break; }
            case OpcodeDef::Mult: {
                Value rhs = callStack.pop();
                Value lhs = callStack.pop();
                lhs.requireType(Value::Integer);
                rhs.requireType(Value::Integer);
                callStack.push(Value{Value::Integer, rhs.value * lhs.value});
                break; }
            case OpcodeDef::Div: {
                Value rhs = callStack.pop();
                Value lhs = callStack.pop();
                lhs.requireType(Value::Integer);
                rhs.requireType(Value::Integer);
                callStack.push(Value{Value::Integer, rhs.value / lhs.value});
                break; }
            case OpcodeDef::Mod: {
                Value rhs = callStack.pop();
                Value lhs = callStack.pop();
                lhs.requireType(Value::Integer);
                rhs.requireType(Value::Integer);
                callStack.push(Value{Value::Integer, rhs.value % lhs.value});
                break; }
            case OpcodeDef::Pow: {
                Value lhs = callStack.pop();
                Value rhs = callStack.pop();
                lhs.requireType(Value::Integer);
                rhs.requireType(Value::Integer);
                int result = 1;
                for (int i = 0; i < rhs.value; ++i) result *= lhs.value;
                callStack.push(Value{Value::Integer, result});
                break; }
            case OpcodeDef::BitLeft: {
                Value v1 = callStack.pop();
                Value v2 = callStack.pop();
                v1.requireType(Value::Integer);
                v2.requireType(Value::Integer);
                callStack.push(Value(Value::Integer, v1.value << v2.value));
                break; }
            case OpcodeDef::BitRight: {
                Value v1 = callStack.pop();
                Value v2 = callStack.pop();
                v1.requireType(Value::Integer);
                v2.requireType(Value::Integer);
                callStack.push(Value(Value::Integer, v1.value >> v2.value));
                break; }
            case OpcodeDef::BitAnd: {
                Value v1 = callStack.pop();
                Value v2 = callStack.pop();
                v1.requireType(Value::Integer);
                v2.requireType(Value::Integer);
                callStack.push(Value(Value::Integer, v1.value & v2.value));
                break; }
            case OpcodeDef::BitOr: {
                Value v1 = callStack.pop();
                Value v2 = callStack.pop();
                v1.requireType(Value::Integer);
                v2.requireType(Value::Integer);
                callStack.push(Value(Value::Integer, v1.value | v2.value));
                break; }
            case OpcodeDef::BitXor: {
                Value v1 = callStack.pop();
                Value v2 = callStack.pop();
                v1.requireType(Value::Integer);
                v2.requireType(Value::Integer);
                callStack.push(Value(Value::Integer, v1.value ^ v2.value));
                break; }
            case OpcodeDef::BitNot: {
                Value v = callStack.pop();
                v.requireType(Value::Integer);
                callStack.push(Value(Value::Integer, ~v.value));
                break; }
            case OpcodeDef::Random: {
                Value max = callStack.pop();
                Value min = callStack.pop();
                min.requireType(Value::Integer);
                max.requireType(Value::Integer);
                int result = min.value + rand() % (max.value - min.value);
                callStack.push(Value{Value::Integer, result});
                break; }
            case OpcodeDef::NextObject: {
                Value lastValue = callStack.pop();
                if (objects.empty()) {
                    callStack.push(noneValue);
                } else {
                    int nextValue = 0;
                    if (lastValue.type != Value::None) {
                        lastValue.requireType(Value::Object);
                        if (lastValue.value > 0) nextValue = lastValue.value;
                    }

                    while (1) {
                        ++nextValue;
                        if (nextValue >= static_cast<int>(objects.size())) {
                            callStack.push(noneValue);
                            break;
                        }
                        try {
                            getObject(nextValue);
                            callStack.push(Value(Value::Object, nextValue));
                            break;
                        } catch (const GameError &e) {
                            // do nothing
                        }
                    }
                }
                break; }
            case OpcodeDef::IndexOf: {
                Value value = callStack.pop();
                Value listId = callStack.pop();
                listId.requireType(Value::List);
                const ListDef &theList = getList(listId.value);
                int result = -1;
                for (unsigned i = 0; i < theList.items.size(); ++i) {
                    if (theList.items[i] == value) {
                        result = i;
                        break;
                    }
                }
                callStack.push(Value(Value::Integer, result));
                break; }
            case OpcodeDef::GetRandom: {
                Value theList = callStack.pop();
                theList.requireType(Value::List);
                const ListDef &listDef = getList(theList.value);
                if (listDef.items.size() == 0) {
                    callStack.push(Value(Value::Integer, 0));
                } else {
                    int choice = rand() % listDef.items.size();
                    callStack.push(listDef.items[choice]);
                }
                break; }
            case OpcodeDef::GetKeys: {
                Value theMap = callStack.pop();
                theMap.requireType(Value::Map);
                const MapDef &mapDef = getMap(theMap.value);
                Value theList = makeNew(Value::List);
                ListDef &listDef = getList(theList.value);
                for (const MapDef::Row &row : mapDef.rows) {
                    listDef.items.push_back(row.key);
                }
                callStack.push(theList);
                break; }

            case OpcodeDef::StackSwap: {
                Value idx1 = callStack.pop();
                Value idx2 = callStack.pop();
                idx1.requireType(Value::Integer);
                idx2.requireType(Value::Integer);
                int stackTop = callStack.getStack().size() - 1;
                Value tmp = callStack.getStack()[stackTop - idx1.value];
                callStack.getStack()[stackTop - idx1.value] = callStack.getStack()[stackTop - idx2.value];
                callStack.getStack()[stackTop - idx2.value] = tmp;
                break; }

            case OpcodeDef::SetSetting: {
                Value settingNumber = callStack.pop();
                Value newValue = callStack.pop();
                settingNumber.requireType(Value::Integer);

                switch(settingNumber.value) {
                    case SETTING_INFOBAR_LEFT:
                        newValue.requireType(Value::String);
                        infoText[INFO_LEFT] = getString(newValue.value).text;
                        break;
                    case SETTING_INFOBAR_RIGHT:
                        newValue.requireType(Value::String);
                        infoText[INFO_RIGHT] = getString(newValue.value).text;
                        break;
                    case SETTING_INFOBAR_FOOTER:
                        newValue.requireType(Value::String);
                        infoText[INFO_BOTTOM] = getString(newValue.value).text;
                        break;
                    case SETTING_INFOBAR_TITLE:
                        newValue.requireType(Value::String);
                        infoText[INFO_TITLE] = getString(newValue.value).text;
                        break;
                }
                break; }

            case OpcodeDef::GetOption: {
                Value extraArg = callStack.pop();
                extraArg.requireType(Value::None, Value::Function);
                optionType = OptionType::Choice;
                callStack.callTop().IP = IP;
                if (extraArg.type == Value::None)   extraValue = -1;
                else                                extraValue = extraArg.value;
                return Value{}; }
            case OpcodeDef::AddOption: {
                Value hotkey = callStack.pop();
                Value extra = callStack.pop();
                Value value = callStack.pop();
                Value text = callStack.pop();
                text.requireType(Value::String);
                hotkey.requireType(Value::Integer, Value::None);
                options.push_back(GameOption{text.value, value, extra,
                                  hotkey.type == Value::None ? -1 : hotkey.value});
                break; }

            case OpcodeDef::StringClear: {
                Value theString = callStack.pop();
                theString.requireType(Value::String);
                StringDef &strDef = getString(theString.value);
                strDef.text.clear();
                break; }
            case OpcodeDef::StringAppend: {
                Value theString = callStack.pop();
                Value toAppend = callStack.pop();
                theString.requireType(Value::String);
                stringAppend(theString, toAppend);
                break; }
            case OpcodeDef::StringAppendUF: {
                Value theString = callStack.pop();
                Value toAppend = callStack.pop();
                theString.requireType(Value::String);
                stringAppend(theString, toAppend, true);
                break; }
            case OpcodeDef::StringLength: {
                Value theString = callStack.pop();
                theString.requireType(Value::String);
                const StringDef &strDef = getString(theString.value);
                int length = strDef.text.size();
                callStack.push(Value{Value::Integer, length});
                break; }
            case OpcodeDef::StringCompare: {
                Value stringA = callStack.pop();
                Value stringB = callStack.pop();
                stringA.requireType(Value::String);
                stringB.requireType(Value::String);
                const StringDef &strADef = getString(stringA.value);
                const StringDef &strBDef = getString(stringB.value);
                callStack.push(Value{Value::Integer,
                        strADef.text != strBDef.text});
                break; }
            case OpcodeDef::Error: {
                Value msg = callStack.pop();
                msg.requireType(Value::String);
                throw GameError(getString(msg.value).text);
                break; }
            case OpcodeDef::Origin: {
                Value ofWhat = callStack.pop();
                std::string text = getSource(ofWhat);
                callStack.push(Value{Value::String, 0});
                break; }
            case OpcodeDef::New: {
                Value type = callStack.pop();
                type.requireType(Value::TypeId);
                callStack.push(makeNew(static_cast<Value::Type>(type.value)));
                break; }
            case OpcodeDef::IsStatic: {
                Value value = callStack.pop();
                callStack.push(Value{Value::Integer,
                                     isStatic(value) ? 1 : 0});
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
