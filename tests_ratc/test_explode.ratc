
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

declare wordList [];
function main() {
    (tokenize "  joy  to  the  world  " wordList none)
    (testStringTokenizeResult wordList tokenizeResultLong "tokenize: maximally spaced")
    (tokenize "   joy   to  the    world  " wordList none)
    (testStringTokenizeResult wordList tokenizeResultLong "failed to explode maximally spaced string")
    (tokenize "joy to the world" wordList none)
    (testStringTokenizeResult wordList tokenizeResultLong "failed to explode minimally spaced string")
    (tokenize "world" wordList none)
    (testStringTokenizeResult wordList tokenizeResultShort "failed to explode single word string")
    (tokenize "  world" wordList none)
    (testStringTokenizeResult wordList tokenizeResultShort "failed to explode single word string with leading whitespace")
    (tokenize "world  " wordList none)
    (testStringTokenizeResult wordList tokenizeResultShort "failed to explode single word string with trailing whitepsace")
    (tokenize "  world  " wordList none)
    (testStringTokenizeResult wordList tokenizeResultShort "failed to explode single word string with leading and trailing whitepsace")
}
