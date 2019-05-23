/* **************************************************************************
 * Expression code generation
 *
 * Generates byte code for expressions in standard-type functions.
 *
 * Part of GTRPE by Gren Drake
 * **************************************************************************/
#include <iostream>
#include <ostream>
#include <sstream>
#include "expression.h"
#include "gamedata.h"
#include "opcode.h"

struct StatementType {
    std::string name;
    void (*handler)(GameData &gamedata, FunctionDef *function, List *list);
};

void handle_asm_stmt(GameData &gamedata, FunctionDef *function, List *list);
void handle_call_stmt(GameData &gamedata, FunctionDef *function, List *list);
void handle_getprop_stmt(GameData &gamedata, FunctionDef *function, List *list);
void handle_reserved_stmt(GameData &gamedata, FunctionDef *function, List *list);
void stmt_print(GameData &gamedata, FunctionDef *function, List *list);
void stmt_label(GameData &gamedata, FunctionDef *function, List *list);

StatementType statementTypes[] = {
    { "label",  stmt_label },
    { "print",  stmt_print },
};


/* ************************************************************************** *
 * General list management functions                                          *
 * ************************************************************************** */

void dump_list(const List *list, std::ostream &out) {
    if (!list) return;
    out << "( ";
    for (const ListValue &v : list->values) {
        if (v.value.type == Value::Expression) {
            dump_list(v.list, out);
        } else {
            out << v.value;
        }
        out << ' ';
    }
    out << ')';
}

bool checkListSize(const List *list, int minSize, int maxSize) {
    if (list->values.size() >= minSize && list->values.size() <= maxSize) {
        return true;
    }
    return false;
}


/* ************************************************************************** *
 * Handlers for statement types                                               *
 * ************************************************************************** */

void handle_asm_stmt(GameData &gamedata, FunctionDef *function, List *list) {
    if (list->values[0].value.type != Value::Opcode) {
        std::stringstream ss;
        ss << "Expected opcode, but found " << list->values[0].value.type << '.';
        gamedata.errors.push_back(Error{list->values[0].origin, ss.str()});
        return;
    }

    const OpcodeDef *opcode = list->values[0].value.opcode;
    int wantedOpcodeCount = opcode->inputs + 1;

    if (opcode->code == OpcodeDef::Call) {
        const int minimumCallOperands = 3;
        if (list->values.size() < minimumCallOperands) {
            std::stringstream ss;
            ss << "Insufficent operands for call opcode (expected at least ";
            ss << minimumCallOperands << ", but found " << list->values.size();
                ss << ").";
            gamedata.errors.push_back(Error{list->values[0].origin, ss.str()});
            checkListSize(list, minimumCallOperands, minimumCallOperands);
            return;
        } else {
            if (list->values[2].value.type != Value::Integer) {
                gamedata.errors.push_back(Error{list->values[2].origin, "Argument count must be integer."});
                return;
            }
            wantedOpcodeCount = minimumCallOperands + list->values[2].value.value;
            if (list->values.size() != wantedOpcodeCount) {
                std::stringstream ss;
                ss << "Insufficent operands for call opcode (expected exactly ";
                ss << wantedOpcodeCount << ", but found " << list->values.size();
                ss << ").";
                gamedata.errors.push_back(Error{list->values[0].origin, ss.str()});
                checkListSize(list, minimumCallOperands, minimumCallOperands);
                return;
            }
        }
    }

    if (!checkListSize(list, wantedOpcodeCount, wantedOpcodeCount)) {
        std::stringstream ss;
        ss << "Opcode expected " << opcode->inputs << " operands, but found " << list->values.size() - 2 << '.';
        gamedata.errors.push_back(Error{list->values[0].origin, ss.str()});
        return;
    }

    for (unsigned i = list->values.size() - 1; i >= 1; --i) {
        const ListValue &theValue = list->values[i];
        if (i == 1 && list->values[0].value.opcode->code == OpcodeDef::Store) {
            if (theValue.value.type != Value::LocalVar) {
                gamedata.errors.push_back(Error{theValue.origin,
                    "Store opcode must reference local variable."});
            } else {
                function->asmCode.push_back(new AsmValue(theValue.origin, Value{Value::VarRef, theValue.value.value}));
            }
        } else {
            if (theValue.value.type == Value::Expression) {
                process_list(gamedata, function, theValue.list);
                if (!gamedata.errors.empty()) return;
            } else {
                function->asmCode.push_back(new AsmValue(theValue.origin, theValue.value));
            }
        }
    }
    function->asmCode.push_back(new AsmOpcode(list->values[0].origin, list->values[0].value.opcode->code));
}

void handle_call_stmt(GameData &gamedata, FunctionDef *function, List *list) {
    const ListValue &func = list->values[0];
    const int argumentCount = list->values.size() - 1;

    for (unsigned i = list->values.size() - 1; i >= 1; --i) {
        const ListValue &theValue = list->values[i];
        if (theValue.value.type == Value::Expression) {
            process_list(gamedata, function, theValue.list);
            if (!gamedata.errors.empty()) return;
        } else {
            function->asmCode.push_back(new AsmValue(theValue.origin, theValue.value));
        }
    }

    function->asmCode.push_back(new AsmValue(func.origin, Value{Value::Integer, argumentCount}));
    if (func.value.type == Value::Expression) {
        process_list(gamedata, function, func.list);
    } else {
        function->asmCode.push_back(new AsmValue(func.origin, func.value));
    }
    function->asmCode.push_back(new AsmOpcode(func.origin, OpcodeDef::Call));
}

void handle_getprop_stmt(GameData &gamedata, FunctionDef *function, List *list) {
    if (list->values.size() != 2) {
        gamedata.errors.push_back(Error{list->values[0].origin,
            "Object property access requires two elements: (object $property)"});
    }
    const ListValue &obj = list->values[0];
    const ListValue &prop = list->values[1];

    if (prop.value.type == Value::Expression) {
        process_list(gamedata, function, prop.list);
    } else {
        function->asmCode.push_back(new AsmValue(prop.origin, prop.value));
    }
    function->asmCode.push_back(new AsmValue(obj.origin, obj.value));
    function->asmCode.push_back(new AsmOpcode(obj.origin, OpcodeDef::GetProp));
}

void handle_reserved_stmt(GameData &gamedata, FunctionDef *function, List *list) {
    const std::string &word = list->values[0].value.text;
    for (const StatementType &stmt : statementTypes) {
        if (stmt.name == word) {
            stmt.handler(gamedata, function, list);
            return;
        }
    }

    std::stringstream ss;
    ss << word << " is not a valid expression command.";
    gamedata.errors.push_back(Error{list->values[0].origin, ss.str()});
}


/* ************************************************************************** *
 * Handlers for reserved words                                                *
 * ************************************************************************** */

void stmt_label(GameData &gamedata, FunctionDef *function, List *list) {
    if (checkListSize(list, 2, 2)) {
        if (list->values[1].value.type != Value::Symbol) {
            std::stringstream ss;
            ss << "Label name must be undefined identifier";
            gamedata.errors.push_back(Error{list->values[1].origin, ss.str()});
        } else {
            function->asmCode.push_back(new AsmLabel(list->values[1].origin, list->values[1].value.text));
        }
    }
}

void stmt_print(GameData &gamedata, FunctionDef *function, List *list) {
    for (unsigned i = 1; i < list->values.size(); ++i) {
        const ListValue &theValue = list->values[i];
        switch(theValue.value.type) {
            case Value::Symbol: {
                std::stringstream ss;
                ss << "Undefined symbol " << theValue.value.text << '.';
                gamedata.errors.push_back(Error{theValue.origin, ss.str()});
                break; }
            case Value::Expression:
                process_list(gamedata, function, theValue.list);
                if (!gamedata.errors.empty()) return;
                break;
            default:
                function->asmCode.push_back(new AsmValue(theValue.origin, theValue.value));
        }
        function->asmCode.push_back(new AsmOpcode(list->values[0].origin, OpcodeDef::Say));
    }
}

/* ************************************************************************** *
 * Core list processing function                                              *
 * ************************************************************************** */

void process_list(GameData &gamedata, FunctionDef *function, List *list) {
    if (!list || list->values.empty()) return;

    switch (list->values[0].value.type) {
        case Value::Node:
        case Value::LocalVar:
        case Value::Expression:
            handle_call_stmt(gamedata, function, list);
            break;
        case Value::Object:
            handle_getprop_stmt(gamedata, function, list);
            break;
        case Value::Opcode:
            handle_asm_stmt(gamedata, function, list);
            break;
        case Value::String:
            list->values.insert(list->values.begin(),
                    ListValue{list->values[0].origin,
                    {Value::Reserved, 0, "print"}});
            stmt_print(gamedata, function, list);
            break;
        case Value::Reserved:
            handle_reserved_stmt(gamedata, function, list);
            break;
        default: {
            std::stringstream ss;
            ss << "Expression not permitted to begin with value of type ";
            ss << list->values[0].value.type << '.';
            gamedata.errors.push_back(Error{list->values[0].origin, ss.str()});
            break; }
    }
}