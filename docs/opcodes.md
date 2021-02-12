---
title: RatCode Opcodes
---

## RatCode Opcodes

Opcodes are divided into three categories: standard, assembly, and restricted.
Standard opcodes may be directly invoked as functions in expressions.
Assembly opcodes may only be used inside of assembly code blocks.
Restricted opcodes may never be explicitly used; they are only ever used implicitly by the compiler.

The opcode entries below are listed in the following format:

**This document is a work in progress and is not yet complete.**

`result name(##) (Argument...)`

where `result` is the type the opcode pushes onto the stack when it completes, `name` is the name of the opcode when used in source files, `##` is the opcode number, and `Argument...` is a list of the types of the arguments that this opcode will require.
All opcodes require that all their arguments to be provided every time they are invoked.


### Standard

`Bool is_valid(18) (Any)`

Determines if the passed value is a valid reference (i.e. if it refers to an value that exists).
This will always return true for primitives types such as integers.

`Any get(22) (List, Integer)`  \
`Any get(22) (Map, Any)`  \
`Any get(22) (Object, Property)`

Retrieves the value of a property, index, or key in an object, list, or map respectively.


### Assembly

Assembly opcodes are those that can only be used during inside of an `asm` statement.
Attempting to use them outside of this context will result in a compiler error.
In many cases, these opcodes have statement equivelents that stand in for them while presenting a more user-friendly syntax.

`-- ret(0) (Any)`

Stops executing the current function and returns passed value to the caller.
Outside of `asm` statements, use the `return` statement instead.

`None say(10) (Any)`

Print the specified value.
Outside of `asm` statements, use the `print` statement instead.

`None pop(13) (Any)`

Removes the top value from the stack and discards it.

`Any stack_dup(14) (Any)`

Duplicates the top value on the stack.
This will cause a runtime error if the stack is empty.

`Any stack_peek(15) (Integer)`

Pushes a copy from the stack position specified by the integer.
Attempting to peek at an invalid index will cause a runtime error.
The provided index is the stack position before the result is pushed.

`Integer stack_size(16) ()`

Finds the number of items on the stack and pushes the result onto the stack.
The provided size is the size prior to result being pushed.

`Any call(17) (Function, Integer)`

Calls the function provided and passes it the specified number of values from the stack as arguments.
This will cause a runtime error if the stack does not have enough values.


`jmp(36)`                       OpcodeDef::Jump,                    1, 0, FORBID_EXPRESSION },
`jz(37)`                        OpcodeDef::JumpZero,                2, 0, FORBID_EXPRESSION },
`jnz(38)`                       OpcodeDef::JumpNotZero,             2, 0, FORBID_EXPRESSION },
`add_option(63)`                OpcodeDef::AddOption,               4, 0, FORBID_EXPRESSION },



### Restricted

Currently, the only opcodes that may never be invoked by a source file are those that push a value onto the stack.
Rather, the author should state the raw value in the source and the appropriate push opcode will be selected automatically.
This is true even in an assembly context.

The opcodes are: `push_0(1)`, `push_1(2)`, `push_none(3)`, `push_8(4)`, `push_16(5)`, and `push_32(6)`.
They never require any arguments and will always push a single value onto the stack.





set",          OpcodeDef::Store,                   2, 0 },
collect",      OpcodeDef::CollectGarbage,          0, 0 },
say_uf",       OpcodeDef::SayUCFirst,              1, 0 },
say_unsigned", OpcodeDef::SayUnsigned,             1, 0 },
say_char",     OpcodeDef::SayChar,                 1, 0 },
is_valid",     OpcodeDef::IsValid,                 1, 1 },
list_push",    OpcodeDef::ListPush,                2, 0 },
list_pop",     OpcodeDef::ListPop,                 1, 1 },
sort",         OpcodeDef::Sort,                    1, 0 },
get",          OpcodeDef::GetItem,                 2, 1 },
has",          OpcodeDef::HasItem,                 2, 1 },
setp",         OpcodeDef::SetItem,                 3, 0 },
size",         OpcodeDef::GetSize,                 1, 1 },
del",          OpcodeDef::DelItem,                 2, 0 },
ins",          OpcodeDef::InsItem,                 3, 0 },
typeof",       OpcodeDef::TypeOf,                  1, 1 },
astype",       OpcodeDef::AsType,                  2, 1 },
eq",           OpcodeDef::Equal,                   2, 1 },
neq",          OpcodeDef::NotEqual,                2, 1 },
lt",           OpcodeDef::LessThan,                2, 1 },
lte",          OpcodeDef::LessThanEqual,           2, 1 },
gt",           OpcodeDef::GreaterThan,             2, 1 },
gte",          OpcodeDef::GreaterThanEqual,        2, 1 },
not",          OpcodeDef::Not,                     1, 1 },
add",          OpcodeDef::Add,                     2, 1 },
sub",          OpcodeDef::Sub,                     2, 1 },
mult",         OpcodeDef::Mult,                    2, 1 },
div",          OpcodeDef::Div,                     2, 1 },
mod",          OpcodeDef::Mod,                     2, 1 },
pow",          OpcodeDef::Pow,                     2, 1 },
left_shift",   OpcodeDef::BitLeft,                 2, 1 },
right_shift",  OpcodeDef::BitRight,                2, 1 },
bit_and",      OpcodeDef::BitAnd,                  2, 1 },
bit_or",       OpcodeDef::BitOr,                   2, 1 },
bit_xor",      OpcodeDef::BitXor,                  2, 1 },
bit_not",      OpcodeDef::BitNot,                  1, 1 },
random",       OpcodeDef::Random,                  2, 1 },
next_object",  OpcodeDef::NextObject,              1, 1 },
indexof",      OpcodeDef::IndexOf,                 2, 1 },
get_random",   OpcodeDef::GetRandom,               1, 1 },
get_keys",     OpcodeDef::GetKeys,                 1, 1 },
stack_swap",   OpcodeDef::StackSwap,               2, 0 },
get_setting",  OpcodeDef::GetSetting,              1, 1 },
set_setting",  OpcodeDef::SetSetting,              2, 0 },
get_key",      OpcodeDef::GetKey,                  1, 1 },
get_option",   OpcodeDef::GetOption,               1, 1 },
get_line",     OpcodeDef::GetLine,                 1, 1 },
str_clear",    OpcodeDef::StringClear,             1, 0 },
str_append",   OpcodeDef::StringAppend,            2, 0 },
str_append_uf",OpcodeDef::StringAppendUF,          2, 0 },
str_length",   OpcodeDef::StringLength,            1, 1 },
str_compare",  OpcodeDef::StringCompare,           2, 1 },
error",        OpcodeDef::Error,                   1, 0 },
origin",       OpcodeDef::Origin,                  1, 1 },
add_page",     OpcodeDef::AddPage,                 3, 0 },
del_page",     OpcodeDef::DelPage,                 1, 0 },
end_page",     OpcodeDef::EndPage,                 0, 0 },
new",          OpcodeDef::New,                     1, 1 },
is_static",    OpcodeDef::IsStatic,                1, 1 },
encode_string",OpcodeDef::EncodeString,            1, 1 },
decode_string",OpcodeDef::DecodeString,            1, 1 },
file_list",    OpcodeDef::FileList,                1, 1 },
file_read",    OpcodeDef::FileRead,                2, 1 },
file_write",   OpcodeDef::FileWrite,               2, 1 },
file_delete",  OpcodeDef::FileDelete,              1, 1 },
tokenize",     OpcodeDef::Tokenize,                1, 1 },




Store               = 7,
CollectGarbage      = 8,
SayUCFirst          = 9,
SayUnsigned         = 11,
SayChar             = 12,
IsValid             = 18,
ListPush            = 19,
ListPop             = 20,
Sort                = 21,
GetItem             = 22, // get item from list (index) or map (key)
HasItem             = 23, // check if index (for list) or key (for map) exists
GetSize             = 24, // get size of list or map
SetItem             = 25, // set item in list (by index) of map (by key)
TypeOf              = 26, // get value type
DelItem             = 27, // remove an item from a list or a key from a map
InsItem             = 28, // inserts an item into a list in the specified position
AsType              = 29, // type conversion

Equal               = 30, // compare two values and push the result
NotEqual            = 31, // compare two values and push the negated result
LessThan            = 32, // jump if top of stack < 0
LessThanEqual       = 33, // jump if top of stack <= 0
GreaterThan         = 34, // jump if top of stack > 0
GreaterThanEqual    = 35, // jump if top of stack >= 0


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
NextObject          = 53,
IndexOf             = 54,
GetRandom           = 55,
GetKeys             = 56,
StackSwap           = 57,
GetSetting          = 58,
SetSetting          = 59,
GetKey              = 60,
GetOption           = 61,
GetLine             = 62,
StringClear         = 65,
StringAppend        = 66,
StringLength        = 67,
StringCompare       = 68,
Error               = 69,
Origin              = 70,
AddPage             = 71,
DelPage             = 72,
EndPage             = 73,
New                 = 74,
StringAppendUF      = 75,
IsStatic            = 76,
EncodeString        = 77,
DecodeString        = 78,
FileList            = 79,
FileRead            = 80,
FileWrite           = 81,
FileDelete          = 82,
Tokenize            = 83,
