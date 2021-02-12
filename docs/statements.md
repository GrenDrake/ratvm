---
title: RatCode Statements
---

## RatCode Statements

Statements resemble opcodes, but can only be used in non-assembly contexts.
Typically they break down into larger chunks of assembly when processed.


`Any asm (special)`

Takes a series of assembly opcodes and commands as its arguments.


`Any (function name) (...)`

A function may be called by beginning a list with the name of the function.
The arguments are passed directly to the function, and the function's return value is returned.


`Bool and (...)`

Takes two or more arguments.
If all arguments are truthy, returns true.
Otherwise, returns false.


`break ()`

Escape the current loop


`Bool continue ()`

Jump to the end of the current loop iteration.


`Bool dec (local-var [amount])`

Decrements the value stored in *local-var* by *amount*.
If *amount* is not provided, the value is decremented by one.


`do_while (condition expression)`

Continually runs *expression* as long as *condition* remains true.


`Bool if (condition true-expression [false-expression])`

Evaluates *condition*.
If it results in a truthy value, evaluate *true-expression*
Otherwise, evaluate *false-expression* if provided.


`Bool inc (local-var [amount])`

Increments the value stored in *local-var* by *amount*.
If *amount* is not provided, the value is incremented by one.


`List list (...)`

Create a new list value using the provided arguments as the initial list items.


`None return (value)`

Return from the current function, passing *value* back to the caller.


`Bool string (...)`

Create a new string value using the provided arguments as the initial string text.


`Bool option (text value [extra] [hotkey])`

Adds an option to the list of options for the current node.
The option will be displayed as *text*.
**get_option** will return *value* if the option is chosen and set the extra value to *extra*.
Passing *hotkey* will set the option to use a specific hotkey instead of being a numbered option.

If *extra* is not provided, it is set to none.
If *hotkey* is not provided, then a designated hotkey will not be set.


`Bool or (...)`

Takes two or more arguments.
Returns true if any of them are truthy.
Returns false otherwise.


`Bool print (...)`

Prints the arguments provided to the screen.


`Bool print_uf (...)`

Prints the arguments provided to the screen.
The first letter of the first argument will be capitalized.


`Bool proc (...)`

Evaluate all passed arguments in sequence.
The result is the value the last argument evaluates to.


`Bool while (condition expression)`

Continually runs *expression* as long as *condition* remains true.
