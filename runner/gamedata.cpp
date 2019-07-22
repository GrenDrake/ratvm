#include <iostream>
#include <sstream>
#include <string>
#include "gamedata.h"

std::string formatText(const std::string &text);

Value MapDef::get(const Value &key) const {
    for (const Row &row : rows) {
        if (row.key == key) return row.value;
    }
    return Value(Value::Integer, 0);
}

bool MapDef::has(const Value &key) const {
    for (const Row &row : rows) {
        if (row.key == key) return true;
    }
    return false;
}

void MapDef::set(const Value &key, const Value &value) {
    for (Row &row : rows) {
        if (row.key == key) {
            row.value = value;
            return;
        }
    }
    rows.push_back(Row{key, value});
}

void MapDef::del(const Value &key) {
    auto iter = rows.begin();
    while (iter != rows.end()) {
        if (iter->key == key) {
            rows.erase(iter);
            return;
        }
        ++iter;
    }
}


Value ObjectDef::get(unsigned propId) const {
    auto iter = properties.find(propId);
    if (iter == properties.end()) return Value{Value::Integer, 0};
    Value result = iter->second;
    result.selfObj = ident;
    return result;
}

bool ObjectDef::has(unsigned propId) const {
    auto iter = properties.find(propId);
    if (iter == properties.end()) return false;
    return true;
}

void ObjectDef::set(unsigned propId, const Value &value) {
    properties[propId] = value;
}


std::string GameData::getSource(const Value &value) {
    std::string text;
    const DataItem *item = nullptr;

    switch(value.type) {
        case Value::None:
        case Value::Integer:
        case Value::Property:
        case Value::String:
        case Value::LocalVar:
        case Value::VarRef:
        case Value::JumpTarget:
            return "";
        case Value::Map:
            item = &getMap(value.value);
            break;
        case Value::List:
            item = &getList(value.value);
            break;
        case Value::Object:
            item = &getObject(value.value);
            break;
        case Value::Function:
            item = &getFunction(value.value);
            break;
    }

    if (item->srcFile == -1) return "no debug info";
    if (item->srcFile == -2) return "dynamic";

    if (item->srcName >= 0) text = "\"" + getString(item->srcName).text + "\" ";
    if (item->srcLine >= 0) text += getString(item->srcFile).text
                                  + ":" + std::to_string(item->srcLine);
    else                    text += getString(item->srcFile).text;

    return text;
}

Value GameData::runFunction(unsigned functionId, std::vector<Value> argList) {
    argList.insert(argList.begin(), noneValue);
    Value returnValue = noneValue;

    optionType = OptionType::None;
    options.clear();
    textBuffer.clear();

    returnValue = runFunctionCore(functionId, argList);

    std::cout << formatText(textBuffer);
    if (textBuffer.back() != '\n') std::cout << '\n';

    switch(optionType) {
        case OptionType::Choice: {
            std::cout << '\n';
            int index = 1;
            for (GameOption &option : options) {
                if (option.hotkey > 0) {
                    std::cout << static_cast<char>(option.hotkey) << ") ";
                    std::cout << strings[option.strId].text << '\n';
                } else {
                    std::cout << index << ") ";
                    std::cout << strings[option.strId].text << '\n';
                    option.hotkey = -index;
                    ++index;
                }
            }
            break; }
        default:
            ;
    }

    ++mCallCount;
    // handle garbage collection
    return returnValue;
}

void GameData::say(const std::string &what) {
    textBuffer += what;
}

void GameData::say(const Value &what) {
    switch(what.type) {
        case Value::String:
            textBuffer += strings[what.value].text;
            break;
        case Value::Integer:
            textBuffer += std::to_string(what.value);
            break;
        default: {
            std::stringstream ss;
            ss << '<' << what.type << ": " << what.value << '>';
            textBuffer += ss.str();
        }
    }
}

