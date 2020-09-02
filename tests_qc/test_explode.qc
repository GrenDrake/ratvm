
declare TITLE   "Automated Test Suite for Tokenize Command";
declare AUTHOR  "Gren Drake";
declare VERSION 1;
declare GAMEID  "";

declare tokenizeResultLong  [ "joy" "to" "the" "world" ];
declare tokenizeResultShort [ "world" ];

function testStringTokenizeResult(result expected message) {
    [ i ]
    (if (neq (size result) (size expected))
        (error message))
    (set i 0)
    (while (lt i (size result))
        (proc
            (if (str_compare (get result i) (get expected i))
                (error message))
            (inc i)))
}

function main() {
    (testStringTokenizeResult (tokenize "  joy  to  the  world  ")     tokenizeResultLong "tokenize: maximally spaced")
    (testStringTokenizeResult (tokenize "   joy   to  the    world  ") tokenizeResultLong "failed to explode maximally spaced string")
    (testStringTokenizeResult (tokenize "joy to the world")            tokenizeResultLong "failed to explode minimally spaced string")

    (testStringTokenizeResult (tokenize "world")     tokenizeResultShort "failed to explode single word string")
    (testStringTokenizeResult (tokenize "  world")   tokenizeResultShort "failed to explode single word string with leading whitespace")
    (testStringTokenizeResult (tokenize "world  ")   tokenizeResultShort "failed to explode single word string with trailing whitepsace")
    (testStringTokenizeResult (tokenize "  world  ") tokenizeResultShort "failed to explode single word string with leading and trailing whitepsace")
}
