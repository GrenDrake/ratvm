#include <algorithm>
#include <ostream>
#include <iomanip>

#include "gamedata.h"
#include "opcode.h"

static void dump_string(std::ostream &out, const std::string &text) {
    const unsigned length = text.size();
    for (unsigned i = 0; i < length; ++i) {
        if (text[i] == '\n') {
            out << "\\n";
        } else {
            out << text[i];
        }
    }
}

void dump_gamedata(GameData &gamedata, std::ostream &out, bool functionBytecode) {
    out << "      SECTION  START      DECIMAL      SIZE (BYTES)\n"
           "  ------------ ---------- ------------ ------------\n"
           "       HEADER: 0x00000000 0            12\n" << std::uppercase;
    out << "      STRINGS: 0x";
    out.fill('0');
    out << std::right << std::hex << std::setw(8) << gamedata.stringsStart;
    out.fill(' ');
    out << ' ' << std::left << std::setw(12) << std::dec << gamedata.stringsStart;
    out << ' ' << gamedata.listsStart - gamedata.stringsStart << '\n';
    out << "        LISTS: 0x";
    out.fill('0');
    out << std::right << std::hex << std::setw(8) << gamedata.listsStart;
    out.fill(' ');
    out << ' ' << std::left << std::setw(12) << std::dec << gamedata.listsStart;
    out << ' ' << gamedata.mapsStart - gamedata.listsStart << '\n';
    out << "         MAPS: 0x";
    out.fill('0');
    out << std::right << std::hex << std::setw(8) << gamedata.mapsStart;
    out.fill(' ');
    out << ' ' << std::left << std::setw(12) << std::dec << gamedata.mapsStart;
    out << ' ' << gamedata.objectsStart - gamedata.mapsStart << '\n';
    out << "      OBJECTS: 0x";
    out.fill('0');
    out << std::right << std::hex << std::setw(8) << gamedata.objectsStart;
    out.fill(' ');
    out << ' ' << std::left << std::setw(12) << std::dec << gamedata.objectsStart;
    out << ' ' << gamedata.functionsStart - gamedata.objectsStart << '\n';
    out << "    FUNCTIONS: 0x";
    out.fill('0');
    out << std::right << std::hex << std::setw(8) << gamedata.functionsStart;
    out.fill(' ');
    out << ' ' << std::left << std::setw(12) << std::dec << gamedata.functionsStart;
    out << ' ' << gamedata.bytecodeStart - gamedata.functionsStart << '\n';
    out << "     BYTECODE: 0x";
    out.fill('0');
    out << std::right << std::hex << std::setw(8) << gamedata.bytecodeStart;
    out.fill(' ');
    out << ' ' << std::left << std::setw(12) << std::dec << gamedata.bytecodeStart;
    out << ' ' << gamedata.fileEnd - gamedata.bytecodeStart << '\n';
    out << "  END-OF-FILE: 0x";
    out.fill('0');
    out << std::right << std::hex << std::setw(8) << gamedata.fileEnd;
    out.fill(' ');
    out << ' ' << std::left << std::dec << gamedata.fileEnd << "\n\n";
    out << std::right << std::dec;

    for (unsigned i = 0; i < gamedata.propertyNames.size(); ++i) {
        out << "PROPERTY-ID " << i << " " << gamedata.propertyNames[i] << '\n';
    }
    out << '\n';

    for (unsigned i = 0; i < gamedata.lists.size(); ++i) {
        const GameList *list = gamedata.lists[i];
        if (list == nullptr) continue;
        out << "LIST " << list->globalId << ": @ " << list->origin << "\n   ";
        for (unsigned i = 0; i < list->items.size(); ++i) {
            out << ' ' << list->items[i];
        }
        out << '\n';
    }
    out << '\n';

    for (unsigned i = 0; i < gamedata.maps.size(); ++i) {
        const GameMap *map = gamedata.maps[i];
        if (map == nullptr) continue;
        out << "MAP " << map->globalId << ": @ " << map->origin << "\n";
        for (unsigned i = 0; i < map->rows.size(); ++i) {
            out << "    " << map->rows[i].key << " => " << map->rows[i].value << '\n';
        }
    }
    out << '\n';

    for (const GameObject *object : gamedata.objects) {
        if (object == nullptr) continue;
        out << "OBJECT: ";
        out << object->globalId << ' ';
        if (object->name.empty())   out << "(anonymous)";
        else                        out << object->name;
        out << " @ " << object->origin << "\n";
        for (const GameProperty &prop : object->properties) {
            out << "    " << prop.id << ": " << prop.value << '\n';
        }
    }
    out << '\n';

    for (unsigned i = 0; i < gamedata.functions.size(); ++i) {
        const FunctionDef *function = gamedata.functions[i];
        if (function == nullptr) continue;
        out << "FUNCTION " << function->globalId << ": ";
        if (function->name.empty()) out << "(anonymous)";
        else                        out << function->name;
        out << " @ " << function->origin << "\n";
        out << "    ARGUMENTS:";
        for (int i = 0; i < function->argument_count; ++i) {
            out << ' ' << function->local_names[i];
        }
        out << "\n    LOCALS:";
        for (int i = 0; i < function->local_count; ++i) {
            out << ' ' << function->local_names[i + function->argument_count];
        }
        out << "\n    TOKEN COUNT: " << function->tokens.size();
        out << "\n    CODE POSITION: " << function->codePosition << " (0x";
        out << std::hex << function->codePosition << std::dec << ')';
        out << "\n    CODE SIZE: " << function->codeEndPosition - function->codePosition;
        out << "\n    CODE END POSITION: " << function->codeEndPosition << " (0x";
        out << std::hex << function->codeEndPosition << std::dec << ')';
        if (functionBytecode) {
            out << "\n    BYTE CODE:";
            function->code.dump(out, 8);
        } else {
            out << '\n';
        }
    }
    out << '\n';

    for (const SymbolDef &symbol : gamedata.symbols.symbols) {
        out << "SYMBOL " << symbol.name << " = " << symbol.value;
        out << " @ " << symbol.origin << '\n';
    }
}

void dump_asm(GameData &gamedata, std::ostream &out) {
    out << std::left;
    unsigned lastOpcodeEntry = 0;
    unsigned longestOpcodeName = 0;
    while(!opcodes[lastOpcodeEntry].name.empty()) {
        if (opcodes[lastOpcodeEntry].name.size() > longestOpcodeName) {
            longestOpcodeName = opcodes[lastOpcodeEntry].name.size();
        }
        ++lastOpcodeEntry;
    }


    for (const FunctionDef *function : gamedata.functions) {
        if (!function) continue;
        out << function->name << " (#" << function->globalId << ") Arguments: ";
        out << function->argument_count << " Locals: " << function->local_count;
        out << " Total: " << function->argument_count + function->local_count << '\n';
        for (unsigned int i = 0; i < function->code.size();) {
            int opcode = function->code.read_8(i++);
            OpcodeDef *def = getOpcodeByCode(opcode);
            ++def->count;
            if (def == nullptr) {
                out << "    (unknown opcode " << opcode << ")\n";
            } else {
                out << "    " << std::setw(longestOpcodeName) << def->name;
                int type = 0, value = 0;
                switch(opcode) {
                    case OpcodeDef::Push0:
                        type = function->code.read_8(i++);
                        out << ' ' << static_cast<Value::Type>(type);
                        break;
                    case OpcodeDef::Push1:
                        type = function->code.read_8(i++);
                        out << ' ' << static_cast<Value::Type>(type);
                        break;
                    case OpcodeDef::PushNeg1:
                        type = function->code.read_8(i++);
                        out << ' ' << static_cast<Value::Type>(type);
                        break;
                    case OpcodeDef::Push8:
                        type = function->code.read_8(i++);
                        value = function->code.read_8(i++);
                        out << ' ' << value;
                        out << ": " << static_cast<Value::Type>(type);
                        break;
                    case OpcodeDef::Push16:
                        type = function->code.read_8(i++);
                        value = function->code.read_16(i);
                        i += 2;
                        out << ' ' << value;
                        out << ": " << static_cast<Value::Type>(type);
                        break;
                    case OpcodeDef::Push32:
                        type = function->code.read_8(i++);
                        value = function->code.read_32(i);
                        i += 4;
                        out << ' ' << value;
                        out << ": " << static_cast<Value::Type>(type);
                        break;
                }
                out << "\n";
            }
        }
        out << "\n";
    }

    out << "OPCODE FREQUENCY:\n";
    std::sort(opcodes, &opcodes[lastOpcodeEntry],
            [](const OpcodeDef &l, const OpcodeDef &r)
                -> bool { return l.count > r.count; });
    for (unsigned int i = 0; !opcodes[i].name.empty(); ++i) {
        out << "    " << std::setw(longestOpcodeName) << opcodes[i].name << ' ' << opcodes[i].count << '\n';
    }
}

void dump_fullBytecode(GameData &gamedata, std::ostream &out) {
    out << "BYTECODE SEGMENT SIZE: " << gamedata.bytecode.size() << '\n';
    gamedata.bytecode.dump(out, 0);
}

void dump_stringtable(GameData &gamedata, std::ostream &out) {
    for (unsigned i = 0; i < gamedata.stringTable.size(); ++i) {
        out << i << " [" << gamedata.stringTable[i].size() << "] ~";
        dump_string(out, gamedata.stringTable[i]);
        out << "~\n";
    }
}

void dump_token_list(const std::vector<Token> &tokens, std::ostream &out) {
    for (const Token &t : tokens) {
        out << t << "\n";
    }
}
