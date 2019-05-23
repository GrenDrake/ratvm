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

struct StatementType {
    std::string name;
    void (*handler)(GameData &gamedata, FunctionDef *function, List *list);
};

void handle_asm_stmt(GameData &gamedata, FunctionDef *function, List *list);
void stmt_print(GameData &gamedata, FunctionDef *function, List *list);
void stmt_label(GameData &gamedata, FunctionDef *function, List *list);

StatementType statementTypes[] = {
    { "label",  stmt_label },
    { "print",  stmt_print },
};

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
            wantedOpcodeCount = minimumCallOperands + list->values[2].value.value;
            if (list->values.size() != wantedOpcodeCount) {
                std::stringstream ss;
                ss << "Uninsufficent operands for call opcode (expected exactly ";
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

void process_list(GameData &gamedata, FunctionDef *function, List *list) {
    if (!list || list->values.empty()) return;

    switch (list->values[0].value.type) {
        case Value::Opcode: {
            handle_asm_stmt(gamedata, function, list);
            break; }
        case Value::String:
            list->values.insert(list->values.begin(),
                    ListValue{list->values[0].origin, {Value::Reserved, 0, "print"}});
            stmt_print(gamedata, function, list);
            break;
        case Value::Reserved: {
            bool handled = false;
            const std::string &word = list->values[0].value.text;
            for (const StatementType &stmt : statementTypes) {
                if (stmt.name == word) {
                    stmt.handler(gamedata, function, list);
                    handled = true;
                    break;
                }
            }
            if (!handled) {
                std::stringstream ss;
                ss << word << " is not a valid expression command.";
                gamedata.errors.push_back(Error{list->values[0].origin, ss.str()});
            }
            break; }
        default: {
            std::stringstream ss;
            ss << "Expression not permitted to begin with value of type ";
            ss << list->values[0].value.type << '.';
            gamedata.errors.push_back(Error{list->values[0].origin, ss.str()});
            break; }
    }
}