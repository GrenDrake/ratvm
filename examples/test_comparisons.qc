declare TITLE   "Automated Test Suite for Comparisons";
declare AUTHOR  "Gren Drake";
declare VERSION 1;
declare GAMEID  "";

// ////////////////////////////////////////////////////////////////////////////
// Test compare command
// ////////////////////////////////////////////////////////////////////////////
function testCompare() {
    [ testValue ]
    (asm
        "\n# Testing compare opcode\n" say

        "Comparing integer values...[br]" say
        5 5 eq testEqFail jnz
        "Failed 5 == 5." error

        testEqFail:
        5 10 eq testNeq jz
        "Failed 5 != 10." error

        testNeq:
        5 5 neq testNeqFail jz
        "Failed 5 == 5." error

        testNeqFail:
        5 10 neq testCmpLT jnz
        "Failed 5 != 10." error


        testCmpLT:
        5 2 lt testCmpGT jnz
        "Failed 2 < 5." error

        testCmpGT:
        2 5 gt testCmpLTE jnz
        "Failed 5 > 2." error

        testCmpLTE:
        5 2 lte testCmpLTE2 jnz
        "Failed 2 <= 5." error

        testCmpLTE2:
        5 5 lte testCmpGTE jnz
        "Failed 5 <= 5." error

        testCmpGTE:
        2 5 gte testCmpGTE2 jnz
        "Failed 5 >= 2." error

        testCmpGTE2:
        5 5 gte testCmpLtNeg jnz
        "Failed 5 >= 5." error

        testCmpLtNeg:
        5 -5 lt testCmpGtNeg jnz
        "Failed -5 < 5." error

        testCmpGtNeg:
        -5 5 gt testNonInteger jnz
        "Failed 5 > -5." error


        testNonInteger:
        "Testing non-integer comparisons...[br]" say
        none none eq testStrNone jnz
        "Failed none == none." error

        testStrNone:
        "\n" none eq testStrFunc jz
        "Failed String != none." error

        testStrFunc:
        "\n" testCompare eq testStrSelf jz
        "Failed String != Function." error

        testStrSelf:
        "\n" "\n" eq testStrStr jnz
        "Failed String != self." error

        testStrStr:
        "\n" " " eq testWithLocals jz
        "Failed String != String." error

        testWithLocals:
        "Testing using local variables...[br]" say
        5 *testValue set
        testValue 5 eq localLt jnz
        "Failed local == 5." error

        localLt:
        testValue 2 lt localGt jnz
        "Failed 2 < local." error

        localGt:
        2 testValue gt done jnz
        "Failed local > 2." error

        done:
    )
}


function testAndOr() {
    ("\n# Testing and/or expressions\n")
    ("Testing [i]and[/i]...[br]")
    (if (and 1 0) (error "(and 1 0) evalulated to true (should be false)"))
    (if (and 0 1) (error "(and 0 1) evalulated to true (should be false)"))
    (if (and 0 0) (error "(and 0 0) evalulated to true (should be false)"))
    (if (not (and 1 1)) (error "(and 1 1) evalulated to false (should be true)"))
    (if (and 0 0 0) (error "(and 0 0 0) evalulated to true (should be false)"))
    (if (and 1 1 0) (error "(and 1 1 0) evalulated to true (should be false)"))
    (if (not (and 1 1 1)) (error "(and 1 1 1) evalulated to false (should be true)"))

    ("Testing [i]or[/i]...[br]")
    (if (not (or 1 0)) (error "(or 1 0) evalulated to false (should be true)"))
    (if (not (or 0 1)) (error "(or 0 1) evalulated to false (should be true)"))
    (if (or 0 0) (error "(or 0 0) evalulated to true (should be false)"))
    (if (not (or 1 1)) (error "(or 1 1) evalulated to false (should be true)"))
    (if (or 0 0 0) (error "(or 0 0 0) evalulated to true (should be false)"))
    (if (not (or 1 1 0)) (error "(or 1 1 0) evalulated to false (should be true)"))
    (if (not (or 1 1 1)) (error "(or 1 1 1) evalulated to false (should be true)"))
}

function main() {
    (testCompare)
    (testAndOr)
}
