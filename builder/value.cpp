/* **************************************************************************
 * General Class Definitions
 *
 * This file contains definitions and close utilities for several small utility
 * classes used by the compiler.
 *
 * Part of GTRPE by Gren Drake
 * **************************************************************************/
#include <fstream>
#include <sstream>
#include <string>

#include "value.h"

std::ostream& operator<<(std::ostream &out, const Value::Type &type) {
    switch(type) {
        case Value::None:
            out << "None";
            break;
        case Value::Integer:
            out << "Integer";
            break;
        case Value::String:
            out << "String";
            break;
        case Value::Symbol:
            out << "Symbol";
            break;
        case Value::Object:
            out << "Object";
            break;
        case Value::List:
            out << "List";
            break;
        case Value::Map:
            out << "Map";
            break;
        case Value::Function:
            out << "Function";
            break;
        case Value::Property:
            out << "Property";
            break;
        case Value::FlagSet:
            out << "FlagSet";
            break;
        case Value::VarRef:
            out << "VarRef";
            break;
        case Value::JumpTarget:
            out << "Jump Target";
            break;
        case Value::LocalVar:
            out << "LocalVar";
            break;
        case Value::Reserved:
            out << "Reserved Word";
            break;
        case Value::Opcode:
            out << "Opcode";
            break;
        case Value::Expression:
            out << "Expression";
            break;
        case Value::TypeId:
            out << "TypeId";
            break;
        default:
            out << "(unhandled type " << static_cast<int>(type) << ")";
    }
    return out;
}

std::ostream& operator<<(std::ostream &out, const Value &value) {
    std::stringstream ss;
    ss << '<' << value.type;
    if (value.type == Value::Symbol || value.type == Value::Reserved || value.type == Value::Opcode) {
        ss << " ~" << value.text << '~';
    } else if (value.type != Value::None && value.type != Value::Expression) {
        ss << ' ' << value.value;
    }
    ss << '>';
    out << ss.str();
    return out;
}