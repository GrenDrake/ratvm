/* STACK TEST FOR EXPRESSIONS
 *
 * This is a compiler test for QuollVM to ensure that all valid expressions are
 * pushing a value onto the stack, even if that value is "none". Because the
 * "proc" expression will pop the result of every child expression but the last,
 * it is ideal for testing this.
 *
 * Due to the nature of this test, it must be run through an interpreter to be
 * validated, not just compiled. A failure of this test will result in a stack
 * underflow runtime error.
 */

declare TITLE "Syntax Expression Stack Test";
declare AUTHOR "Gren Drake";
declare VERSION 1;
declare GAMEID "23D1A6FF-C456-48E3-8894-879401C2DDB6";

declare aList [ 9 5 6 3 7 8 ];
object anObject $aProperty 55;
declare aMap { 4: 5 2: 5 };

function main() {
    [ local ]
    // set the local variable to an int value so inc/dec will work
    (set local 0)

    // constructed expressions
    ("This test serves to make sure that all expressions leave the stack in a suitable clean state when they finish executing.\n")
    (asm 4 pop)
    (and 0 0) // result false
    (and 1 2) // result true
    (dec local)
    (do_while 0 0)
    (if 0 0)
    (inc local)
    (list 0 0 0)
    (string "Some text\n")
    (option "Some text\n")
    (or 0 0) // result false
    (or 1 1) // result true
    (print "Testing print.[br]")
    (print_uf "Testing print_uf[br]")
    (proc 1 2 3)
    (while 0 1)

    // asm expressions
    (set local (new String))
    ("Testing say_unsigned: ")
    (say_unsigned -1)
    ("[br]Testing say_char: ")
    (say_char 65)
    (list_push aList 5)
    (list_pop aList)
    (sort aList)
    (get aList 1)
    (has aList 1)
    (setp anObject $aProperty 12)
    (size aList)
    (del aList 1)
    (ins aList 3 3)
    (typeof aList)
    (astype aList String)
    (eq 0 1)
    (neq 0 1)
    (lt 4 4)
    (lte 2 5)
    (gt 5 6)
    (gte 4 5)
    (not 0)
    (add 1 1)
    (sub 4 4)
    (mult 3 7)
    (div 3 6)
    (mod 40 5)
    (pow 2 4)
    (left_shift 3 4)
    (right_shift 4 5)
    (bit_and 2 4)
    (bit_or 7 4)
    (bit_not 4)
    (bit_xor 3 4)
    (random 2 5)
    (get_random aList)
    (get_keys aMap)
    // (get_setting 0) // no settings exist to get currently
    (set_setting infobarLeft "Updated text")
    (str_clear local)
    (str_append local "Some text\n")
    (str_append_uf local "Some text\n")
    (str_length local)
    (str_compare local "Some text\n")
    (origin aMap)
    (new List)
    (is_static aMap)
    (encode_string "Hello")
    (decode_string (list))

    // control-flow items
    // these aren't tested because they change the program flow rather than effecting the stack
    // (break)
    // (continue)
    // (error)

    // input-related items
    // these aren't being tested because of difficulty in automation
    // (get_key)
    // (get_option none)
    // (get_line)

    // verify that stack is still empty
    (asm
        stack_size 0 eq stack_is_good jnz
        "Stack not clear at end of function." error
        stack_is_good:)
    ("\nTest passed successfullly.")
}