declare TITLE   "Automated Test Suite for Strings";
declare AUTHOR  "Gren Drake";
declare VERSION 1;
declare GAMEID  "";



// ////////////////////////////////////////////////////////////////////////////
// Test String Manipulation
// ////////////////////////////////////////////////////////////////////////////

// these the two strings each repersent the unicode character "GREEK UPSILON
// WITH ACUTE AND HOOK SYMBOL", but the first string repersents it as a single
// code point while the second repersents it as two code points. Normalization
// should ensure that these end up the same in the final VM image.
declare strYcomposed "ϓ";   // code point:  03D3       UTF8: 0xcf 0x93
declare strYdecomposed "ϓ"; // code points: 03D2 0301  UTF8: 0xcf 0x92 0xcc 0x81
declare strYfirst "ϒ"; // code point: 03D2
declare strYsecond "́"; // code point: 0301
declare strSigmaLower "σ"; // code point 03C2
declare strSigmaUpper "Σ"; // code point 03A3

function testStringManipulation() {
    [ testString ]
    (asm
        "\n# Test String Manipulation\n" say

        "Check static string \"Fred\" has length four...[br]" say
        "Fred" str_length 4 eq strlenNewString jnz
        "Bad length on static string \"Fred\"." error

        strlenNewString:
        String new *testString set
        "Check new string has zero length...[br]" say
        testString str_length strcpyStrlen jz
        "Bad initial length on new string." error

        strcpyStrlen:
        "Copying \"Fred\" to dynamic string...[br]" say
        "Fred" testString str_append
        testString str_length 4 eq stringCompares jnz
        "Bad length on copied string \"Fred\"." error

        stringCompares:
        "Comparing static and dynamic strings with [b]==[/b]...[br]" say
        testString "Fred" eq stringCompareStrcmp jz
        "Identity of dynamic and static strings compared equal." error

        stringCompareStrcmp:
        "Comparing static and dynamic strings with [b]strcmp[/b]...[br]" say
        "Fred" testString str_compare stringCompareNormalized jz
        "Dynamic and static strings did not compare equal." error

        stringCompareNormalized:
        "Testing normalization of static strings...[br]" say
        // makes sure the composed and decomposed forms of the same string are being
        // repersented by the same string object
        Integer strYcomposed astype
        Integer strYdecomposed astype
        eq stringAppend jnz
        "Text incorrectly normallized." error

        stringAppend:
        "Appending to test string...[br]" say
        testString str_clear
        "fred" testString str_append
        " Lives!" testString str_append
        "fred Lives!" testString str_compare stringAppendNormalized jz
        "String append malfunctioned." error

        stringAppendNormalized:
        "Testing append normalization...[br]" say
        testString str_clear
        strYfirst testString str_append
        strYsecond testString str_append
        strYcomposed testString str_compare stringAppendUF jz
        "Appended string not normalized." error

        stringAppendUF:
        "Appending to test string with initial upper...[br]" say
        testString str_clear
        "abc" testString str_append_uf
        "Abc" testString str_compare stringAppendUFExtended jz
        "String append UF malfunctioned." error

        stringAppendUFExtended:
        "Appending to test string with initial upper with larger codepoint...[br]" say
        testString str_clear
        strSigmaLower testString str_append_uf
        strSigmaUpper testString str_compare doneStrings jz
        "str_append_uf failed to uppercase σ to Σ." error

        doneStrings:
        0 testStringBuilder call
    )
}

function testStringBuilder() {
    [ theString ]
    ("Testing string builder...[br]")
    (set theString (string "I ate " 3 " turnips."))
    (if (str_compare theString "I ate 3 turnips.")
        (error "newstr did not create string correctly."))
}


declare testString1 "あいう";
declare testString2 "Hello World!";
function testStringEncoding() {
    [ encodedList recodedText ]
    (print "\n# Testing string en/decoding\n")

    (print "Testing " testString1 "...[br]")
    (set encodedList (encode_string testString1))
    (if (neq (size encodedList) 3) (error "First encoded string has wrong length."))
    (if (neq (get encodedList 0) 0xe38182e3) (error "First encoded string first value incorrect."))
    (if (neq (get encodedList 1) 0x8184e381) (error "First encoded string second value incorrect."))
    (if (neq (get encodedList 2) 0x86000000) (error "First encoded string third value incorrect."))
    (set recodedText (decode_string encodedList))
    (if (str_compare recodedText testString1) (error "Recoded test string one has wrong value."))

    (print "Testing " testString2 "...[br]")
    (set encodedList (encode_string testString2))
    (if (neq (size encodedList) 3) (error "Second encoded string has wrong length."))
    (if (neq (get encodedList 0) 0x48656c6c) (error "Second encoded string first value incorrect."))
    (if (neq (get encodedList 1) 0x6f20576f) (error "Second encoded string second value incorrect."))
    (if (neq (get encodedList 2) 0x726c6421) (error "Second encoded string third value incorrect."))
    (set recodedText (decode_string encodedList))
    (if (str_compare recodedText testString2) (error "Recoded test string two has wrong value."))
}

function main() {
    (testStringManipulation)
    (testStringEncoding)
}
