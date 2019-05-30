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

void handle_asm_stmt(GameData &gamedata, FunctionDef *function, List *list);
void handle_call_stmt(GameData &gamedata, FunctionDef *function, List *list);
void handle_reserved_stmt(GameData &gamedata, FunctionDef *function, List *list);
void stmt_and(GameData &gamedata, FunctionDef *function, List *list);
void stmt_break(GameData &gamedata, FunctionDef *function, List *list);
void stmt_continue(GameData &gamedata, FunctionDef *function, List *list);
void stmt_do_while(GameData &gamedata, FunctionDef *function, List *list);
void stmt_if(GameData &gamedata, FunctionDef *function, List *list);
void stmt_label(GameData &gamedata, FunctionDef *function, List *list);
void stmt_or(GameData &gamedata, FunctionDef *function, List *list);
void stmt_print(GameData &gamedata, FunctionDef *function, List *list);
void stmt_print_uf(GameData &gamedata, FunctionDef *function, List *list);
void stmt_proc(GameData &gamedata, FunctionDef *function, List *list);
void stmt_while(GameData &gamedata, FunctionDef *function, List *list);

void process_value(GameData &gamedata, FunctionDef *function, ListValue &value);

/* ************************************************************************** *
 * Reserved words and statements                                              *
 * ************************************************************************** */

StatementType statementTypes[] = {
    { "",           nullptr       },
    { "and",        stmt_and    },
    { "break",      stmt_break    },
    { "continue",   stmt_continue },
    { "do_while",   stmt_do_while },
    { "if",         stmt_if       },
    { "label",      stmt_label    },
    { "or",         stmt_or       },
    { "print",      stmt_print    },
    { "print_uf",   stmt_print_uf },
    { "proc",       stmt_proc     },
    { "while",      stmt_while    },
};

const StatementType& getReservedWord(const std::string &word) {
    for (const StatementType &stmt : statementTypes) {
        if (stmt.name == word) return stmt;
    }
    return statementTypes[0];
}

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
            return;
        } else {
            if (list->values[2].value.type != Value::Integer) {
                gamedata.errors.push_back(Error{list->values[2].origin, "Argument count must be integer."});
                return;
            }
            wantedOpcodeCount = minimumCallOperands + list->values[2].value.value;
            if (list->values.size() != wantedOpcodeCount) {
                std::stringstream ss;
                ss << "Incorrect number of operands for call opcode (expected exactly ";
                ss << wantedOpcodeCount << ", but found " << list->values.size();
                ss << ").";
                gamedata.errors.push_back(Error{list->values[0].origin, ss.str()});
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
                function->addValue(theValue.origin, Value{Value::VarRef, theValue.value.value});
            }
        } else {
            if (theValue.value.type == Value::Expression) {
                process_list(gamedata, function, theValue.list);
                if (!gamedata.errors.empty()) return;
            } else {
                function->addValue(theValue.origin, theValue.value);
            }
        }
    }
    function->addOpcode(list->values[0].origin, list->values[0].value.opcode->code);
    if (list->values[0].value.opcode->outputs <= 0) {
        function->addValue(list->values[0].origin, Value{Value::None});
    }
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
            function->addValue(theValue.origin, theValue.value);
        }
    }

    function->addValue(func.origin, Value{Value::Integer, argumentCount});
    if (func.value.type == Value::Expression) {
        process_list(gamedata, function, func.list);
    } else {
        function->addValue(func.origin, func.value);
    }
    function->addOpcode(func.origin, OpcodeDef::Call);
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

void stmt_and(GameData &gamedata, FunctionDef *function, List *list) {
    if (list->values.size() < 3) {
        gamedata.errors.push_back(Error{list->values[1].origin, "and requires at least two arguments."});
        return;
    }

    const Origin &origin = list->values[0].origin;
    std::string after_label = "__label_" + std::to_string(function->nextLabel);
    ++function->nextLabel;
    std::string false_label = "__label_" + std::to_string(function->nextLabel);
    ++function->nextLabel;

    for (unsigned i = 1; i < list->values.size(); ++i) {
        process_value(gamedata, function, list->values[i]);
        function->addValue(origin, Value{Value::Symbol, 0, false_label});
        function->addOpcode(list->values[0].origin, OpcodeDef::JumpZero);
    }
    function->addValue(origin, Value{Value::Integer, 1});
    function->addValue(origin, Value{Value::Symbol, 0, after_label});
    function->addOpcode(list->values[0].origin, OpcodeDef::Jump);

    function->addLabel(origin, false_label);
    function->addValue(origin, Value{Value::Integer, 0});
    function->addLabel(origin, after_label);
}

void stmt_or(GameData &gamedata, FunctionDef *function, List *list) {
    if (list->values.size() < 3) {
        gamedata.errors.push_back(Error{list->values[1].origin, "and requires at least two arguments."});
        return;
    }

    const Origin &origin = list->values[0].origin;
    std::string after_label = "__label_" + std::to_string(function->nextLabel);
    ++function->nextLabel;
    std::string true_label = "__label_" + std::to_string(function->nextLabel);
    ++function->nextLabel;

    for (unsigned i = 1; i < list->values.size(); ++i) {
        process_value(gamedata, function, list->values[i]);
        function->addValue(origin, Value{Value::Symbol, 0, true_label});
        function->addOpcode(list->values[0].origin, OpcodeDef::JumpZero);
    }
    function->addValue(origin, Value{Value::Integer, 0});
    function->addValue(origin, Value{Value::Symbol, 0, after_label});
    function->addOpcode(list->values[0].origin, OpcodeDef::Jump);

    function->addLabel(origin, true_label);
    function->addValue(origin, Value{Value::Integer, 1});
    function->addLabel(origin, after_label);
}

void stmt_break(GameData &gamedata, FunctionDef *function, List *list) {
    if (!checkListSize(list, 1, 1)) {
        gamedata.errors.push_back(Error{list->values[1].origin, "break statement cannot take arguments."});
        return;
    }

    if (function->breakLabels.empty()) {
        gamedata.errors.push_back(Error{list->values[1].origin, "break statement found outside loop."});
        return;
    }

    const Origin &origin = list->values[0].origin;
    function->addValue(origin, Value{Value::Symbol, 0, function->breakLabels.back()});
    function->addOpcode(origin, OpcodeDef::Jump);
}

void stmt_continue(GameData &gamedata, FunctionDef *function, List *list) {
    if (!checkListSize(list, 1, 1)) {
        gamedata.errors.push_back(Error{list->values[1].origin, "continue statement cannot take arguments."});
        return;
    }

    if (function->continueLabels.empty()) {
        gamedata.errors.push_back(Error{list->values[1].origin, "continue statement found outside loop."});
        return;
    }

    const Origin &origin = list->values[0].origin;
    function->addValue(origin, Value{Value::Symbol, 0, function->continueLabels.back()});
    function->addOpcode(origin, OpcodeDef::Jump);
}

void stmt_do_while(GameData &gamedata, FunctionDef *function, List *list) {
    if (list->values.size() != 3) {
        gamedata.errors.push_back(Error{list->values[0].origin, "While statement must have three expressions."});
        return;
    }

    std::string start_label = "__label_" + std::to_string(function->nextLabel);
    ++function->nextLabel;
    std::string condition_label = "__label_" + std::to_string(function->nextLabel);
    ++function->nextLabel;
    std::string after_label = "__label_" + std::to_string(function->nextLabel);
    ++function->nextLabel;

    function->continueLabels.push_back(condition_label);
    function->breakLabels.push_back(after_label);

    const Origin &origin = list->values[0].origin;

    function->addLabel(origin, start_label);
    process_value(gamedata, function, list->values[1]);
    function->addLabel(origin, condition_label);
    process_value(gamedata, function, list->values[2]);
    function->addValue(origin, Value{Value::Symbol, 0, after_label});
    function->addOpcode(origin, OpcodeDef::JumpZero);
    function->addValue(origin, Value{Value::Symbol, 0, start_label});
    function->addOpcode(origin, OpcodeDef::Jump);
    function->addLabel(origin, after_label);

    function->continueLabels.pop_back();
    function->breakLabels.pop_back();
}

void stmt_label(GameData &gamedata, FunctionDef *function, List *list) {
    if (checkListSize(list, 2, 2)) {
        if (list->values[1].value.type != Value::Symbol) {
            std::stringstream ss;
            ss << "Label name must be undefined identifier";
            gamedata.errors.push_back(Error{list->values[1].origin, ss.str()});
        } else {
            function->addLabel(list->values[1].origin, list->values[1].value.text);
        }
    }
}

void stmt_if(GameData &gamedata, FunctionDef *function, List *list) {
    if (list->values.size() < 3 || list->values.size() > 4) {
        gamedata.errors.push_back(Error{list->values[0].origin, "If expression must have two or three values."});
        return;
    }

    std::string after_label = "__label_" + std::to_string(function->nextLabel);
    ++function->nextLabel;
    std::string else_label = "__label_" + std::to_string(function->nextLabel);
    ++function->nextLabel;

    const Origin &origin = list->values[0].origin;
    process_value(gamedata, function, list->values[1]);
    function->addValue(origin, Value{Value::Symbol, 0, else_label});
    function->addOpcode(origin, OpcodeDef::JumpZero);
    process_value(gamedata, function, list->values[2]);
    function->addValue(origin, Value{Value::Symbol, 0, after_label});
    function->addOpcode(origin, OpcodeDef::Jump);
    function->addLabel(origin, else_label);
    if (list->values.size() >= 4) {
        process_value(gamedata, function, list->values[3]);
    } else {
        function->addValue(list->values[0].origin, Value{Value::Integer, 0});
    }
    function->addLabel(origin, after_label);
}

void stmt_print(GameData &gamedata, FunctionDef *function, List *list) {
    if (list->values.size() <= 1) {
        gamedata.errors.push_back(Error{list->values[0].origin, "print statement requires arguments."});
        return;
    }
    for (unsigned i = 1; i < list->values.size(); ++i) {
        process_value(gamedata, function, list->values[i]);
        function->addOpcode(list->values[0].origin, OpcodeDef::Say);
    }
}

void stmt_print_uf(GameData &gamedata, FunctionDef *function, List *list) {
    if (list->values.size() <= 1) {
        gamedata.errors.push_back(Error{list->values[0].origin, "print statement requires arguments."});
        return;
    }
    process_value(gamedata, function, list->values[1]);
    function->addOpcode(list->values[0].origin, OpcodeDef::SayUCFirst);

    for (unsigned i = 2; i < list->values.size(); ++i) {
        process_value(gamedata, function, list->values[i]);
        function->addOpcode(list->values[0].origin, OpcodeDef::Say);
    }
}

void stmt_proc(GameData &gamedata, FunctionDef *function, List *list) {
    if (list->values.size() < 2) {
        gamedata.errors.push_back(Error{list->values[0].origin, "proc statement must contain at least one statement."});
        return;
    }

    for (unsigned i = 1; i < list->values.size(); ++i) {
        process_value(gamedata, function, list->values[i]);
    }
}

void stmt_while(GameData &gamedata, FunctionDef *function, List *list) {
    if (list->values.size() != 3) {
        gamedata.errors.push_back(Error{list->values[0].origin, "While statement must have three expressions."});
        return;
    }

    std::string start_label = "__label_" + std::to_string(function->nextLabel);
    ++function->nextLabel;
    std::string after_label = "__label_" + std::to_string(function->nextLabel);
    ++function->nextLabel;

    function->continueLabels.push_back(start_label);
    function->breakLabels.push_back(after_label);

    const Origin &origin = list->values[0].origin;

    function->addLabel(origin, start_label);
    process_value(gamedata, function, list->values[1]);
    function->addValue(origin, Value{Value::Symbol, 0, after_label});
    function->addOpcode(origin, OpcodeDef::JumpZero);
    process_value(gamedata, function, list->values[2]);
    function->addValue(origin, Value{Value::Symbol, 0, start_label});
    function->addOpcode(origin, OpcodeDef::Jump);
    function->addLabel(origin, after_label);

    function->continueLabels.pop_back();
    function->breakLabels.pop_back();
}


/* ************************************************************************** *
 * Core list processing function                                              *
 * ************************************************************************** */

void process_value(GameData &gamedata, FunctionDef *function, ListValue &value) {
    switch(value.value.type) {
        case Value::Reserved:
        case Value::Opcode: {
            std::stringstream ss;
            ss << "Invalid expression value of type " << value.value.type << '.';
            gamedata.errors.push_back(Error{value.origin, ss.str()});
            break; }
        case Value::Symbol: {
            std::stringstream ss;
            ss << "Undefined symbol " << value.value.text << '.';
            gamedata.errors.push_back(Error{value.origin, ss.str()});
            break; }
        case Value::Expression:
            process_list(gamedata, function, value.list);
            if (!gamedata.errors.empty()) return;
            break;
        default:
            function->addValue(value.origin, value.value);
    }
}

void process_list(GameData &gamedata, FunctionDef *function, List *list) {
    if (!list || list->values.empty()) return;

    switch (list->values[0].value.type) {
        case Value::Node:
        case Value::LocalVar:
        case Value::Expression:
            handle_call_stmt(gamedata, function, list);
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