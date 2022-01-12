#include <cctype>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include "gamedata.h"
#include "opcode.h"
#include "textutil.h"
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
                localId.requireType(Value::VarRef);
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
                    upperFirst(toSay);
                    say(toSay);
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
                std::string aString = codepointToString(theText.value);
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
                const auto &args = callStack.getStack().argList;
                for (int i = 0; i < static_cast<int>(args.size()); ++i) {
                    if (newFunc.argTypes[i] != Value::Any && args[i].type != newFunc.argTypes[i]) {
                        const std::string &name = getString(newFunc.srcName).text;
                        std::stringstream ss;
                        ss << "Function " << name << " expected argument ";
                        ss << i << " to be " <<  newFunc.argTypes[i];
                        ss << " but received " << args[i].type;
                        throw GameError(ss.str());
                    }
                }
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
                callStack.push(Value(Value::Integer, static_cast<int>(def.items.size())));
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
                    theIndex.value = static_cast<int>(listDef.items.size());
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
                Value min = callStack.pop();
                Value max = callStack.pop();
                min.requireType(Value::Integer);
                max.requireType(Value::Integer);
                if (min.value == max.value) {
                    return min;
                } else {
                    int maxv = max.value, minv = min.value;
                    if (maxv < minv) {
                        int t = maxv;
                        maxv = minv;
                        minv = t;
                    }
                    int result = minv + rand() % (maxv - minv);
                    callStack.push(Value{Value::Integer, result});
                }
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
                        } catch (const GameError&) {
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
                    std::vector<Value>::size_type choice = rand() % listDef.items.size();
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

            case OpcodeDef::GetKey: {
                Value promptStr = callStack.pop();
                promptStr.requireType(Value::String);
                optionType = OptionType::Key;
                callStack.callTop().IP = IP;
                options.push_back(GameOption{promptStr.value, noneValue, noneValue, -1});
                return Value{}; }
            case OpcodeDef::GetOption: {
                Value extraArg = callStack.pop();
                extraArg.requireType(Value::None, Value::VarRef);
                optionType = OptionType::Choice;
                callStack.callTop().IP = IP;
                if (extraArg.type == Value::None)   extraValue = -1;
                else                                extraValue = extraArg.value;
                return Value{}; }
            case OpcodeDef::GetLine: {
                Value promptStr = callStack.pop();
                promptStr.requireType(Value::String);
                optionType = OptionType::Line;
                callStack.callTop().IP = IP;
                options.push_back(GameOption{promptStr.value, noneValue, noneValue, -1});
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
                callStack.push(makeNewString(text));
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

            case OpcodeDef::EncodeString: {
                Value stringId = callStack.pop();
                stringId.requireType(Value::String);
                std::string str = getString(stringId.value).text;
                Value listId = makeNew(Value::List);
                callStack.push(listId);
                ListDef &list = getList(listId.value);

                unsigned v = 0;
                int counter = 0;
                for (char s : str) {
                    unsigned byte = static_cast<unsigned>(s) & 0xFF;
                    v <<= 8;
                    v |= byte;
                    ++counter;
                    if (counter >= 4) {
                        list.items.push_back(Value(Value::Integer, v));
                        counter = v = 0;
                    }
                }
                if (counter != 0) {
                    while (counter < 4) {
                        ++counter;
                        v <<= 8;
                    }
                    list.items.push_back(Value(Value::Integer, v));
                }
                break; }
            case OpcodeDef::DecodeString: {
                Value listId = callStack.pop();
                listId.requireType(Value::List);
                const ListDef &list = getList(listId.value);
                std::string result;
                for (const Value &value : list.items) {
                    value.requireType(Value::Integer);
                    unsigned v4 = (value.value >> 24) & 0xFF;
                    if (v4 == 0) break;
                    result += static_cast<char>(v4);
                    unsigned v3 = (value.value >> 16) & 0xFF;
                    if (v3 == 0) break;
                    result += static_cast<char>(v3);
                    unsigned v2 = (value.value >> 8) & 0xFF;
                    if (v2 == 0) break;
                    result += static_cast<char>(v2);
                    unsigned v1 = value.value & 0xFF;
                    if (v1 == 0) break;
                    result += static_cast<char>(v1);
                }
                Value stringId = makeNew(Value::String);
                getString(stringId.value).text = result;

                callStack.push(stringId);
                break; }

            case OpcodeDef::FileList: {
                Value gameIdRef = callStack.pop();
                gameIdRef.requireType(Value::String, Value::None);
                std::string forGameId;
                std::string myGameId = "";
                if (gameIdRef.type != Value::None) {
                    forGameId = getString(gameIdRef.value).text;
                    myGameId = getString(refGameid).text;
                }
                FileList filelist = getFileList();
                Value listId = makeNew(Value::List);
                ListDef &list = getList(listId.value);
                callStack.push(listId);
                for (auto record : filelist) {
                    if (forGameId != myGameId) continue;
                    Value rowId = makeNew(Value::List);
                    ListDef &row = getList(rowId.value);
                    row.items.push_back(makeNewString(record.name));
                    std::time_t recordDate = record.date;
                    std::string timeString = trim(ctime(&recordDate));
                    row.items.push_back(makeNewString(timeString));
                    row.items.push_back(makeNewString(record.gameId));
                    list.items.push_back(rowId);
                }
                break; }
            case OpcodeDef::FileRead: {
                Value fileNameId = callStack.pop();
                fileNameId.requireType(Value::String);
                const std::string &filename = getString(fileNameId.value).text;
                Value listId = getFile(filename);
                callStack.push(listId);
                break; }
            case OpcodeDef::FileWrite: {
                Value fileNameId = callStack.pop();
                Value dataListId = callStack.pop();
                fileNameId.requireType(Value::String);
                dataListId.requireType(Value::List);
                const std::string &filename = getString(fileNameId.value).text;
                const ListDef &listDef = getList(dataListId.value);
                bool result = saveFile(filename, &listDef);
                callStack.push(Value{Value::Integer, result ? 1 : 0});
                break; }
            case OpcodeDef::FileDelete: {
                Value fileNameId = callStack.pop();
                fileNameId.requireType(Value::String);
                const std::string &filename = getString(fileNameId.value).text;
                bool result = deleteFile(filename);
                callStack.push(Value{Value::Integer, result ? 1 : 0});
                break; }

            case OpcodeDef::Tokenize: {
                Value text = callStack.pop();
                Value strList = callStack.pop();
                Value vocabList = callStack.pop();
                text.requireType(Value::String);
                strList.requireType(Value::List, Value::None);
                vocabList.requireType(Value::List, Value::None);
                ListDef *strListDef = strList.type == Value::None ? nullptr : &getList(strList.value);
                if (strListDef) strListDef->items.clear();
                ListDef *vocabListDef = vocabList.type == Value::None ? nullptr : &getList(vocabList.value);
                if (vocabListDef) vocabListDef->items.clear();

                auto result = explodeString(getString(text.value).text);
                for (std::string word : result) {
                    strToLower(word);
                    if (strListDef)     strListDef->items.push_back(makeNewString(word));
                    if (vocabListDef)   vocabListDef->items.push_back(Value(Value::Vocab, getVocab(word)));
                }
                break; }

            case OpcodeDef::GetChildCount: {
                Value objectId = callStack.pop();
                objectId.requireType(Value::Object);
                const ObjectDef &object = getObject(objectId.value);
                if (object.childId == 0) {
                    callStack.push(Value{Value::Integer, 0});
                } else {
                    int count = 0;
                    const ObjectDef *child = &getObject(object.childId);
                    while (1) {
                        ++count;
                        if (child->siblingId == 0) break;
                        child = &getObject(child->siblingId);
                    }
                    callStack.push(Value{Value::Integer, count});
                }
                break; }
            case OpcodeDef::GetParent: {
                Value objectId = callStack.pop();
                objectId.requireType(Value::Object);
                const ObjectDef &object = getObject(objectId.value);
                if (object.parentId == 0) {
                    callStack.push(noneValue);
                } else {
                    callStack.push(Value{Value::Object, object.parentId});
                }
                break; }
            case OpcodeDef::GetFirstChild: {
                Value objectId = callStack.pop();
                objectId.requireType(Value::Object);
                const ObjectDef &object = getObject(objectId.value);
                if (object.childId == 0) {
                    callStack.push(noneValue);
                } else {
                    callStack.push(Value{Value::Object, object.childId});
                }
                break; }
            case OpcodeDef::GetSibling: {
                Value objectId = callStack.pop();
                objectId.requireType(Value::Object);
                const ObjectDef &object = getObject(objectId.value);
                if (object.siblingId == 0) {
                    callStack.push(noneValue);
                } else {
                    callStack.push(Value{Value::Object, object.siblingId});
                }
                break; }
            case OpcodeDef::GetChildren: {
                Value objectId = callStack.pop();
                objectId.requireType(Value::Object);
                Value childList = makeNew(Value::List);
                callStack.push(childList);
                const ObjectDef &object = getObject(objectId.value);
                if (object.childId != 0) {
                    ListDef &list = getList(childList.value);
                    const ObjectDef *child = &getObject(object.childId);
                    while (1) {
                        list.items.push_back(Value{Value::Object, static_cast<int>(child->ident)});
                        if (child->siblingId == 0) break;
                        child = &getObject(child->siblingId);
                    }
                }
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
