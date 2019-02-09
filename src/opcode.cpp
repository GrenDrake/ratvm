#include "opcode.h"

OpcodeDef opcodes[] = {
    {   "return",       OpcodeDef::Return       },
    {   "push_0",       OpcodeDef::Push0        },
    {   "push_1",       OpcodeDef::Push1        },
    {   "push_n1",      OpcodeDef::PushNeg1     },
    {   "push_8",       OpcodeDef::Push8        },
    {   "push_16",      OpcodeDef::Push16       },
    {   "push_32",      OpcodeDef::Push32       },
    {   "store",        OpcodeDef::Store        },
    {   "say_uf",       OpcodeDef::SayUCFirst   },
    {   "say",          OpcodeDef::Say          },
    {   "say_unsigned", OpcodeDef::SayUnsigned  },
    {   "say_char",     OpcodeDef::SayChar      },
    {   "pop",          OpcodeDef::StackPop     },
    {   "stack_dup",    OpcodeDef::StackDup     },
    {   "stack_peek",   OpcodeDef::StackPeek    },
    {   "stack_size",   OpcodeDef::StackSize    },
    {   "call",         OpcodeDef::Call         },
    {   "call_method",  OpcodeDef::CallMethod   },
    {   "self",         OpcodeDef::Self         },
    {   "get_prop",     OpcodeDef::GetProp      },
    {   "has_prop",     OpcodeDef::HasProp      },
    {   "set_prop",     OpcodeDef::SetProp      },
    {   "get_item",     OpcodeDef::GetItem      },
    {   "has_item",     OpcodeDef::HasItem      },
    {   "get_size",     OpcodeDef::GetSize      },
    {   "set_item",     OpcodeDef::SetItem      },
    {   "del_item",     OpcodeDef::DelItem      },
    {   "add_item",     OpcodeDef::AddItem      },
    {   "typeof",       OpcodeDef::TypeOf       },
    {   "astype",       OpcodeDef::AsType       },
    {   "cmp",          OpcodeDef::Compare              },
    {   "jmp",          OpcodeDef::Jump                 },
    {   "jeq",          OpcodeDef::JumpZero             },
    {   "jz",           OpcodeDef::JumpZero             },
    {   "jneq",         OpcodeDef::JumpNotZero          },
    {   "jnz",          OpcodeDef::JumpNotZero          },
    {   "jlt",          OpcodeDef::JumpLessThan         },
    {   "jlte",         OpcodeDef::JumpLessThanEqual    },
    {   "jgt",          OpcodeDef::JumpGreaterThan      },
    {   "jgte",         OpcodeDef::JumpGreaterThanEqual },
    {   "not",          OpcodeDef::Not                  },
    {   "add",          OpcodeDef::Add          },
    {   "sub",          OpcodeDef::Sub          },
    {   "mult",         OpcodeDef::Mult         },
    {   "div",          OpcodeDef::Div          },
    {   "mod",          OpcodeDef::Mod},
    {   "pow",          OpcodeDef::Pow},
    {   "left_shift",   OpcodeDef::BitLeft      },
    {   "right_shift",  OpcodeDef::BitRight     },
    {   "bit_and",      OpcodeDef::BitAnd       },
    {   "bit_or",       OpcodeDef::BitOr        },
    {   "bit_xor",      OpcodeDef::BitXor       },
    {   "bit_not",      OpcodeDef::BitNot       },
    {   "random",       OpcodeDef::Random       },
    {   "inc",          OpcodeDef::Inc          },
    {   "dec",          OpcodeDef::Dec          },
    {   "get_random",   OpcodeDef::GetRandom    },
    {   "get_keys",     OpcodeDef::GetKeys      },
    {   "stack_swap",   OpcodeDef::StackSwap    },
    {   "get_setting",  OpcodeDef::GetSetting   },
    {   "set_setting",  OpcodeDef::SetSetting   },
    {   "get_key",      OpcodeDef::GetKey       },
    {   "get_option",   OpcodeDef::GetOption    },
    {   "get_line",     OpcodeDef::GetLine      },
    {   "add_option",   OpcodeDef::AddOption    },
    {   "add_option_x", OpcodeDef::AddOptionExtra },
    {   "strcpy",       OpcodeDef::StringCopy   },
    {   "strcat",       OpcodeDef::StringAppend },
    {   "strlen",       OpcodeDef::StringLength },
    {   "strcmp",       OpcodeDef::StringCompare },
    {   "error",        OpcodeDef::Error        },
    {   "add_page",     OpcodeDef::AddPage      },
    {   "del_page",     OpcodeDef::DelPage      },
    {   "end_page",     OpcodeDef::EndPage      },
    {   "new",          OpcodeDef::New          },
    {   "is_static",    OpcodeDef::IsStatic     },
    {   ""                                      }
};

const OpcodeDef* getOpcode(const std::string &name) {
    for (const OpcodeDef &code : opcodes) {
        if (code.name == name) return &code;
    }
    return nullptr;
}

OpcodeDef* getOpcodeByCode(int codeNumber) {
    for (OpcodeDef &code : opcodes) {
        if (code.code == codeNumber) return &code;
    }
    return nullptr;
}
