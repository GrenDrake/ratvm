// ////////////////////////////////////////////////////////////////////////////
// First string : this must be the first string used for one of the tests
//                to function correctly.
// ////////////////////////////////////////////////////////////////////////////
declare firstString "this is the first string";


declare TITLE   "Automated Test Suite for Values";
declare AUTHOR  "Gren Drake";
declare VERSION 1;
declare GAMEID  "";


// ////////////////////////////////////////////////////////////////////////////
// Data used for tests
// ////////////////////////////////////////////////////////////////////////////

declare testInt 7384;
declare testStr "A Name";
declare testProperty $testFunc;
declare testFlags flags( 6 7 8 testInt );
declare testList [ 1 2 3 ];
declare testMap {
    1: "The"
    2: "End"
};
object first_obj
    $anInt 2
    $aString "Hello World!"
    $aProperty $apple
    $anObject object ;
;

// ////////////////////////////////////////////////////////////////////////////
// Test default values commands
// ////////////////////////////////////////////////////////////////////////////
default undeclaredValue 5;
default declaredValue 10;
declare declaredValue 20;
declare otherDeclaredValue 40;
default otherDeclaredValue 30;
default aFunction testDefaults;

function testDefaults() {
    ("\n# Testing default values\n")
    (if (neq undeclaredValue 5) (error "undeclaredValue has wrong value"))
    (if (neq declaredValue 20) (error "declaredValue has wrong value"))
    (if (neq otherDeclaredValue 40) (error "otherDeclaredValue has wrong value"))
    (if (neq aFunction testDefaults) (error "aFunction has wrong value"))
}

// ////////////////////////////////////////////////////////////////////////////
// Test value types
// ////////////////////////////////////////////////////////////////////////////
function testValues() {
    [ localvar ]
    ("\n# Testing Value Types\n")

    ("Checking basic value types...[br]")
    (if (neq (typeof testInt) Integer)       (error "Failed typeof testInt == Integer."))
    (if (neq (typeof testStr) String)        (error "Failed typeof testStr == String."))
    (if (neq (typeof testList) List)         (error "Failed typeof testList == List."))
    (if (neq (typeof testMap) Map)           (error "Failed typeof testMap == Map."))
    (if (neq (typeof testValues) Function)   (error "Failed typeof testValues == Function."))
    (if (neq (typeof first_obj) Object)      (error "Failed typeof first_obj == Object."))
    (if (neq (typeof testProperty) Property) (error "Failed typeof testProperty == Property."))
    (if (neq (typeof testFlags) Integer)     (error "Failed typeof testFlags == Integer."))
    (if (neq localvar none)                  (error "Failed localvar default value == none."))
    (set localvar 0)
    (if (neq (typeof localvar) Integer)      (error "Failed typeof localvar == Integer."))
    (set localvar (astype localvar String))
    (if (neq localvar firstString)           (error "Failed localvar cast to string."))

    (asm
        *localvar typeof Reference eq all_done jnz
        "Failed typeof *localvar == Reference." error
        all_done:
    )
}

// ////////////////////////////////////////////////////////////////////////////
// Test isvalid
// ////////////////////////////////////////////////////////////////////////////
function testIsValid() {
    [ localvar ]
    (asm
        "\n# Testing is_valid\n" say

        test_isvalid:
        "Testing basic types...[br]" say
        none is_valid false eq test_isvalid_badnone jnz
        5 is_valid false eq test_isvalid_badint jnz
        $property is_valid false eq test_isvalid_badproperty jnz
        test_isvalid is_valid false eq test_isvalid_badlabel jnz
        *localvar is_valid false eq test_isvalid_badvarref jnz
        Integer is_valid false eq test_isvalid_badtype jnz

        "Testing invalid 0 values...[br]" say
        Function 0 astype is_valid false eq test_isvalid_funczero jz
        List 0 astype is_valid false eq test_isvalid_listzero jz
        Map 0 astype is_valid false eq test_isvalid_mapzero jz
        Object 0 astype is_valid false eq test_isvalid_objectzero jz

        "Testing valid values...[br]" say
        "The" is_valid false eq test_isvalid_badstring jnz
        testList is_valid false eq test_isvalid_badlist jnz
        testMap is_valid false eq test_isvalid_badmap jnz
        first_obj is_valid false eq test_isvalid_badobject jnz
        testValues is_valid false eq test_isvalid_badfunc jnz

        "Testing invalid values...[br]" say
        String 99999 astype is_valid false eq test_isvalid_badbadstring jz
        List 99999 astype is_valid false eq test_isvalid_badbadlist jz
        Map 99999 astype is_valid false eq test_isvalid_badbadmap jz
        Object 99999 astype is_valid false eq test_isvalid_badbadobject jz
        Function 99999 astype is_valid false eq test_isvalid_badbadfunc jz

        ret

        test_isvalid_badnone:
            "None registered invalid." error
        test_isvalid_badint:
            "Integer 5 registered invalid." error
        test_isvalid_badproperty:
            "Property $property registered invalid." error
        test_isvalid_badlabel:
            "Label test_isvalid registered invalid." error
        test_isvalid_badvarref:
            "VarRef localvar registered invalid." error
        test_isvalid_badtype:
            "Integer type registered invalid." error

        test_isvalid_funczero:
            "Function 0 registered as valid." error
        test_isvalid_listzero:
            "List 0 registered as valid." error
        test_isvalid_mapzero:
            "Map 0 registered as valid." error
        test_isvalid_objectzero:
            "Object 0 registered as valid." error

        test_isvalid_badstring:
            "String \"The\" registered as invalid." error
        test_isvalid_badlist:
            "List testList registered as invalid." error
        test_isvalid_badmap:
            "Map testMap registered as invalid." error
        test_isvalid_badobject:
            "Object first_obj registered as invalid." error
        test_isvalid_badfunc:
            "Function testValues registered as invalid." error

        test_isvalid_badbadstring:
            "Invalid String registered as invalid." error
        test_isvalid_badbadlist:
            "Invalid List registered as invalid." error
        test_isvalid_badbadmap:
            "Invalid Map registered as invalid." error
        test_isvalid_badbadobject:
            "Invalid Object registered as invalid." error
        test_isvalid_badbadfunc:
            "Invalid Function registered as invalid." error
    )
}

// ////////////////////////////////////////////////////////////////////////////
// Tests runner
// ////////////////////////////////////////////////////////////////////////////
function main() {
    (testDefaults)
    (testValues)
    (testIsValid)

    ("\nALL VALUE TESTS PASSED")
}
