#ifndef OPCODE_H
#define OPCODE_H

#include <string>

struct OpcodeDef {
    enum CodeOpcode {
        Return              = 0,
        Push0               = 1,
        Push1               = 2,
        PushNone            = 3,
        Push8               = 4,
        Push16              = 5,
        Push32              = 6,
        Store               = 7,
        SayUCFirst          = 9,
        Say                 = 10,
        SayUnsigned         = 11,
        SayChar             = 12,
        StackPop            = 13, // remove the top item from the stack
        StackDup            = 14, // duplicate the top item on the stack
        StackPeek           = 15, // peek at the stack item X items from the top
        StackSize           = 16, // get the current size of the stack
        Call                = 17, // call a value as a function
        CallMethod          = 18, // call an object property as a function
        Self                = 19, // get object the current function is a property of
        GetProp             = 20,
        HasProp             = 21, // check if property is set on object
        SetProp             = 22, // set object property to value
        GetItem             = 23, // get item from list (index) or map (key)
        HasItem             = 24, // check if index (for list) or key (for map) exists
        GetSize             = 25, // get size of list or map
        SetItem             = 26, // set item in list (by index) of map (by key)
        TypeOf              = 27, // get value type
        DelItem             = 28, // remove an item from a list or a key from a map
        AddItem             = 29, // add an item to a list (use set-item for maps)
        AsType              = 30, // type conversion
        Compare             = 31, // compare two values and push the result
        Jump                = 32, // unconditional jump
        JumpZero            = 33, // jump if top of stack == 0
        JumpNotZero         = 34, // jump if top of stack != 0
        JumpLessThan        = 35, // jump if top of stack < 0
        JumpLessThanEqual   = 36, // jump if top of stack <= 0
        JumpGreaterThan     = 37, // jump if top of stack > 0
        JumpGreaterThanEqual= 38, // jump if top of stack >= 0
        Not                 = 39,
        Add                 = 40,
        Sub                 = 41,
        Mult                = 42,
        Div                 = 43,
        Mod                 = 44,
        Pow                 = 45,
        BitLeft             = 46,
        BitRight            = 47,
        BitAnd              = 48,
        BitOr               = 49,
        BitXor              = 50,
        BitNot              = 51,
        Random              = 52,
        Inc                 = 53,
        Dec                 = 54,
        GetRandom           = 55,
        GetKeys             = 56,
        StackSwap           = 57,
        GetSetting          = 58,
        SetSetting          = 59,
        GetKey              = 60,
        GetOption           = 61,
        GetLine             = 62,
        AddOption           = 63,
        AddOptionExtra      = 64,
        StringCopy          = 65,
        StringAppend        = 66,
        StringLength        = 67,
        StringCompare       = 68,
        Error               = 69,
        /* unused: 70 */
        AddPage             = 71,
        DelPage             = 72,
        EndPage             = 73,
        New                 = 74,
        /* Unused opcode: 75 */
        IsStatic            = 76,
    };

    std::string name;
    int code;
    int inputs;
    int outputs;

    int count;
};

const OpcodeDef* getOpcode(const std::string &name);
OpcodeDef* getOpcodeByCode(int codeNumber);

extern OpcodeDef opcodes[];

#endif
