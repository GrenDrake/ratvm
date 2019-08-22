// ////////////////////////////////////////////////////////////////////////////
// First string : this must be the first string used for one of the tests
//                to function correctly.
// ////////////////////////////////////////////////////////////////////////////
declare firstString "this is the first string";


// ////////////////////////////////////////////////////////////////////////////
// Data used for tests
// ////////////////////////////////////////////////////////////////////////////
declare testInt 7384;
declare testStr "A Name";
declare testList [ 1 2 3 ];
declare testMap {
    1: "The"
    2: "End"
};
declare testProperty $testFunc;
declare testFlags flags( 6 7 8 testInt );

extend testList [ 100 200 ];
extend testMap {
    3: "Is"
    4: "Far"
};

object first_obj
    $anInt 2
    $aString "Hello World!"
    $aProperty $apple
    $anObject object ;
    $aMap { $fruit: $apple }
    $aList [ 4 ]
    $testMethod function() { }
;
object second_obj : parent_obj;

object parent_obj
    $inheritedProperty 2048
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
// Test stack commands
// ////////////////////////////////////////////////////////////////////////////
asm_function testStack() {
    2 4 6 8

    "\n# Testing stack manipulation\n" say

    "Testing correct initial size...[br]" say
    stack_size 4 eq testPeek jnz
    "Stack has bad initial size." error

    testPeek:
    "Testing stack peeking...[br]" say
    0 stack_peek 8 eq testPeek2 jnz
    "Failed peek at top item." error
    testPeek2:
    3 stack_peek 2 eq testPop jnz
    "Failed peek at bottom item." error

    testPop:
    "Popping item and checking stack size...[br]" say
    pop
    stack_size 3 eq testPush jnz
    "Stack has bad size after pop." error

    testPush:
    "Pushing new item and checking stack size...[br]" say
    94
    stack_size 4 eq testDup jnz
    "Stack has bad size after push." error

    testDup:
    "Duplicating top item and checking size and value...[br]" say
    stack_dup
    stack_size 5 eq testDup2 jnz
    "Stack has bad size after stack_dup." error
    testDup2:
    eq testSwap jnz
    "Duplicate item not equal to original." error

    testSwap:
    "Testing stack swapping...[br]" say
    9 7 5 // add some dummy values
    0 2 stack_swap
    0 stack_peek 9 eq testSwapError jz
    2 stack_peek 5 eq testSwapError jz

    return

    testSwapError:
    "Stack swap failed." error
}


// ////////////////////////////////////////////////////////////////////////////
// Test function calls and arguments
// ////////////////////////////////////////////////////////////////////////////
asm_function testCall() {
    "\n# Testing function calls\n" say

    "Checking value of self on non-method...[br]" say
    self none eq oneUnspecified jnz
    "Incorrect value of self." error

    oneUnspecified:
    "Testing call with unspecified arguments...[br]" say
    24 1 testCall_onlyOne call

    "Testing call with extra arguments...[br]" say
    34 88 24 3 testCall_both call

    "Checking result of calling testCallHelperMultiply(5,6)...[br]" say
    5 6 2 testCallHelperMultiply call 30 eq testMethodCall jnz
    "testCallHelperMultiply(5,6) did not have result of 30." error

    testMethodCall:
    "Calling testCallObject.testMethod...[br]" say
    3 2 1 3 $testMethod testCallObject get call

    "Testing return values of non-asm functions.[br]" say
    0 testSyntaxReturn call 5 eq testReturnDefaultLabel jnz
    "testSyntaxReturn returned wrong value." error

    testReturnDefaultLabel:
    0 testSyntaxReturnDefault call 0 eq alldone jnz
    "testSyntaxReturnDefault returned wrong value." error

    alldone:
}

object testCallObject
    $testMethod asm_function(arg1 arg2 arg3) {
        "[i]testMethod:[/i] Checking value of self...[br]" say
        self testCallObject eq testArg1 jnz
        "Incorrect value of self." error

        testArg1:
        "[i]testMethod:[/i] Checking argument values...[br]" say
        arg1 1 eq testArg2 jnz
        "First argument has wrong value." error
        testArg2:
        arg2 2 eq testArg3 jnz
        "Second argument has wrong value." error
        testArg3:
        arg3 3 eq done jnz
        "Third argument has wrong value." error

        done:
    }
;

asm_function testCall_onlyOne( arg1 arg2 ) {
    arg1 none eq compareArg2 jz
    "First passed argument had unexpected none" error

    compareArg2:
    arg2 none eq done jnz
    "Second passed argument not none" error

    done:
}

asm_function testCall_both( arg1 arg2 : var1 ) {
    arg1 none eq compareArg2 jz
    "First passed argument had unexpected none" error

    compareArg2:
    arg2 none eq compareVar1 jz
    "First passed argument had unexpected none" error

    compareVar1:
    var1 none eq done jnz
    "First non-argument local not none" error

    done:
}

asm_function testCallHelperMultiply( arg1 arg2 ) {
    arg1 arg2 mult return
}

function testSyntaxReturn() {
    (return 5)
}
function testSyntaxReturnDefault() {
    // uses default return value
}

// ////////////////////////////////////////////////////////////////////////////
// Test compare command
// ////////////////////////////////////////////////////////////////////////////
asm_function testCompare(: testValue) {
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
}


// ////////////////////////////////////////////////////////////////////////////
// Test jump commands
// ////////////////////////////////////////////////////////////////////////////
asm_function testJumps(: testValue) {
    "\n# Testing jump opcodes\n" say

    "Testing unconditional jump...[br]" say
    test001 jmp
    "Failed unconditional jump." error

    test001:
    "Testing conditional jumps...[br]" say
    0 test002 jz
    "Failed jz (0)." error

    test002:
    1 test002_failed jz
    test003 jmp
    test002_failed:
    "Failed jz (1)." error

    test003:
    0 test003_failed jnz
    test004 jmp
    test003_failed:
    "Failed jnz (0)." error

    test004:
    1 test017 jnz
    "Failed jnz (1)." error


    test017:
    "Test jump target in local variable...[br]" say
    test018 *testValue set
    testValue jmp
    "Failed jmp w/ local." error

    test018:
    test019 *testValue set
    0 testValue jz
    "Failed jz w/ local." error

    test019:
    test024 *testValue set
    1 testValue jnz
    "Failed jnz w/ local." error

    test024:
    "Testing with value in local variable...[br]" say
    0 *testValue set
    testValue test025 jz
    "Failed jz w/ value in local." error

    test025:
    1 *testValue set
    testValue done jnz
    "Failed jnz w/ value in local." error

    done:
}


// ////////////////////////////////////////////////////////////////////////////
// Test math commands
// ////////////////////////////////////////////////////////////////////////////
asm_function testMath(: counter) {
    "\n# Testing math opcodes\n" say
    "Testing arithmetic...[br]" say

    5 5 add 10 eq testMinus jnz
    "Failed 5 + 5 == 10." error

    testMinus:
    5 10 sub 5 eq testMult jnz
    "Failed 10 - 5 == 5." error

    testMult:
    10 5 mult 50 eq testDiv jnz
    "Failed 10 * 5 == 50." error

    testDiv:
    5 40 div 8 eq testMod jnz
    "Failed 40 / 5 == 8." error

    testMod:
    6 40 mod 4 eq testPow jnz
    "Failed 40 % 6 == 4." error

    testPow:
    8 2 pow 256 eq testLeftShift jnz
    "Failed 2 ^ 8 == 256." error

    testLeftShift:
    "Testing bit operations...[br]" say
    4 1 left_shift 16 eq testRightShift jnz
    "Failed 1 << 4 == 16." error

    testRightShift:
    4 32 right_shift 2 eq testBitAnd jnz
    "Failed 32 >> 4 == 2." error

    testBitAnd:
    90 170 bit_and 10 eq testBitOr jnz
    "Failed 90 & 170 == 10." error

    testBitOr:
    90 170 bit_or 250 eq testBitXor jnz
    "Failed 90 | 170 == 250." error

    testBitXor:
    90 170 bit_xor 240 eq testBitNot jnz
    "Failed 90 ^ 170 == 240." error

    testBitNot:
    148 bit_not -149 eq testTruth jnz
    "Failed ~148 == -149." error

    testTruth:
    "Testing logical not...[br]" say
    0 not testNotOne jnz
    "Failed !0 == true." error

    testNotOne:
    1 not testNot37 jz
    "Failed !1 == false." error

    testNot37:
    37 not testNotNeg9 jz
    "Failed !37 == false." error

    testNotNeg9:
    -9 not testNotFred jz
    "Failed !-9 == false." error

    testNotFred:
    "Fred" not testNotEmptyString jz
    "Failed !\"Fred\" == false." error

    testNotEmptyString:
    "" not testNotFunction jz
    "Failed !\"\" == false." error

    testNotFunction:
    testMath not testNotNone jz
    "Failed !testMath() == false." error

    testNotNone:
    none not done jnz
    "Failed !none == true." error

    done:
}


// ////////////////////////////////////////////////////////////////////////////
// Test object property commands
// ////////////////////////////////////////////////////////////////////////////
asm_function testObjectProps() {
    "\n# Testing Object Manipulation\nTesting get on objects...[br]" say
    $aString first_obj get "Hello World!" eq object_get jnz
    "first_obi.aString did not equal \"Hello World!\"" error

    object_get:
    $anInt first_obj get 2 eq noexistant_property jnz
    "first_obj.anInt did not equal 2" error

    noexistant_property:
    $aString second_obj get 0 eq object_has jnz
    "second_obj.aString (non-existant) did not equal 0" error

    object_has:
    "Testing has on objects...[br]" say
    $anInt first_obj has has_astring jnz
    "first_obj denies having $anInt" error

    has_astring:
    $aString second_obj has test_setp jz
    "second_obj claims to have $aString" say

    test_setp:
    "Testing setp on objects...[br]" say
    "Fred" $apple second_obj setp
    $apple second_obj get "Fred" eq test_inherited jnz
    "failed to update second_obj.$apple" error

    test_inherited:
    "Testing inherited properties...[br]" say
    $inheritedProperty second_obj has 0 eq inherited_has_prop jz
    $inheritedProperty second_obj get 2048 eq inherited_wrong_value jz
    $inheritedProperty first_obj get 0 eq noparent_doesnt_have jz
    $doesNotExist second_obj get 0 eq parent_doesnt_have jz

    "Testing property types[br]" say
    $anInt      first_obj get typeof Integer    eq anInt_wrongType jz
    $aString    first_obj get typeof String     eq aString_wrongType jz
    $aMap       first_obj get typeof Map        eq aMap_wrongType jz
    $testMethod first_obj get typeof Function   eq testMethod_wrongType jz
    $anObject   first_obj get typeof Object     eq anObject_wrongType jz
    $aProperty  first_obj get typeof Property   eq aProperty_wrongType jz
    0 return

    inherited_has_prop:     "HAS reports object own parent's property" error
    inherited_wrong_value:  "inherited property returns wrong value" error
    noparent_doesnt_have:   "unparented object returns wrong value for non-existant property" error
    parent_doesnt_have:     "parented object returns wrong value for non-existant property" error
    anInt_wrongType:        "first_obj.$anInt has wrong type." error
    aString_wrongType:      "first_obj.$aString has wrong type." error
    aList_wrongType:        "first_obj.$aList has wrong type." error
    aMap_wrongType:         "first_obj.$aMap has wrong type." error
    testMethod_wrongType:   "first_obj.$testMethod has wrong type." error
    anObject_wrongType:     "first_obj.$anObject has wrong type." error
    aProperty_wrongType:    "first_obj.$aProperty has wrong type." error
}


// ////////////////////////////////////////////////////////////////////////////
// Test value types
// ////////////////////////////////////////////////////////////////////////////
asm_function testValues(: localvar) {
    "\n# Testing Value Types\n" say

    "Checking basic value types...[br]" say
    testInt typeof Integer eq test_string jnz
    "Failed typeof testInt == Integer." error

    test_string:
    testStr typeof String eq test_list jnz
    "Failed typeof testStr == String." error

    test_list:
    testList typeof List eq test_map jnz
    "Failed typeof testList == List." error

    test_map:
    testMap typeof Map eq test_function jnz
    "Failed typeof testMap == Map." error

    test_function:
    testValues typeof Function eq test_object jnz
    "Failed typeof testValues == Function." error

    test_object:
    first_obj typeof Object eq test_property jnz
    "Failed typeof first_obj == testObject." error

    test_property:
    testProperty typeof Property eq test_varref jnz
    "Failed typeof testProperty == Property." error

    test_varref:
    *localvar typeof Reference eq test_flags jnz
    "Failed typeof *localvar == Reference." error

    test_flags:
    testFlags typeof Integer eq test_local jnz
    "Failed typeof testFlags == Integer." error

    test_local:
    "Testing local variable values...[br]" say
    localvar none eq test_local_2 jnz
    "Failed localvar default value == none." error

    test_local_2:
    0 *localvar set
    localvar typeof Integer eq test_casts jnz
    "Failed typeof localvar == Integer." error

    test_casts:
    "Testing type casting...[br]" say
    String localvar astype *localvar set
    localvar firstString eq all_done jnz
    "Failed localvar cast to string." error

    all_done:
}


// ////////////////////////////////////////////////////////////////////////////
// Test List Methods
// ////////////////////////////////////////////////////////////////////////////
asm_function printArray(theArray : counter length) {
    "[b]Array contents:[/b]" say
    theArray size *length set
    0 *counter set
    arraydumploop:
    " " say counter theArray get say
    counter 1 add *counter set
    length counter lt arraydumploop jnz
    " (length: " say length say ")\n" say
}

asm_function testLists(: counter random_value) {
    "\n# Testing List Operations\n" say

    "Testing list insertion...[br]" say
    testList size 5 eq test_add jnz
    "Test list has wrong initial size." error

    test_add:
    91 99 testList ins
    testList size 6 eq test_add_initial jnz
    "Test list has wrong size after add at end." error

    test_add_initial:
    46 0 testList ins
    testList size 7 eq test_add_middle jnz
    "Test list has wrong size after add at beginning." error

    test_add_middle:
    13 2 testList ins
    testList size 8 eq test_list_contents jnz
    "Test list has wrong size after add in middle." error

    test_list_contents:
    0 testList get 46 eq test_list_contents_wrong jz
    1 testList get  1 eq test_list_contents_wrong jz
    2 testList get 13 eq test_list_contents_wrong jz
    3 testList get  2 eq test_list_contents_wrong jz
    4 testList get  3 eq test_list_contents_wrong jz
    5 testList get 100 eq test_list_contents_wrong jz
    6 testList get 200 eq test_list_contents_wrong jz
    7 testList get 91 eq test_list_contents_wrong jz


    "Testing list pop/push...[br]" say
    1024 testList list_push
    testList size 9 eq test_list_push_value jnz
    "list_push did not increase list size." error

    test_list_push_value:
    6 testList get 1024 neq test_list_pop jz
    "Pushed list item has wrong value"

    test_list_pop:
    testList list_pop *counter set
    testList size 8 eq test_list_pop_value jnz
    "list_pop did not decrease list size." error

    test_list_pop_value:
    1024 counter eq test_list_set jnz
    "list_pop returned wrong value" error


    test_list_set:
    "Testing changing value in list...[br]" say
    second_obj 2 testList setp
    2 testList get second_obj eq test_deletion jnz
    "Failed to update list index." error


    test_deletion:
    "Testing list deletion...[br]" say
    2 testList del
    testList size 7 eq test_del_first jnz
    "Test list has wrong size after delete." error

    test_del_first:
    0 testList del
    testList size 6 eq test_del_last jnz
    "Test list has wrong size after delete." error

    test_del_last:
    5 testList del
    testList size 5 eq test_list_contents_2 jnz
    "Test list has wrong size after delete." error

    test_list_contents_2:
    0 testList get  1 eq test_list_contents_wrong jz
    1 testList get  2 eq test_list_contents_wrong jz
    2 testList get  3 eq test_list_contents_wrong jz
    3 testList get  100 eq test_list_contents_wrong jz
    4 testList get  200 eq test_list_contents_wrong jz


    "Testing bad list indexes...[br]" say
    -1 testList get 0 eq test_list_badindex jnz
    "Accessing negative list index does not return 0." error

    test_list_badindex:
    99 testList get 0 eq test_random jnz
    "Accessing list index beyond list size does not return 0." error


    test_random:
    "Testing getting random list items...[br]" say
    0 *counter set
    randomloop:
    testList get_random *random_value set
    random_value 1 eq randomloop_good jnz
    random_value 2 eq randomloop_good jnz
    random_value 3 eq randomloop_good jnz
    random_value 100 eq randomloop_good jnz
    random_value 200 eq randomloop_good jnz
    "Got unexpected value when drawing random list values." error
    randomloop_good:
    counter 1 add *counter set
    200 counter gt randomloop jnz

    0 testListSort call
    0 testListBuilder call
    return

    test_list_contents_wrong:
    "\n" say testList 1 printArray call
    "List has wrong contents." error
    return
}
function testListSort(: theList) {
    ("Testing list sorting...[br]")
    (set theList (new List))
    (list_push theList 5)
    (list_push theList 10)
    (list_push theList 0)
    (sort theList)
    (if (neq (get theList 0) 0) (error "First sorted list item wrong."))
    (if (neq (get theList 1) 5) (error "Second sorted list item wrong."))
    (if (neq (get theList 2) 10) (error "Third sorted list item wrong."))

    (set theList (new List))
    (list_push theList "leg")
    (list_push theList "arm")
    (list_push theList "tail")
    (list_push theList "head")
    (sort theList)
    (if (neq (get theList 0) "arm") (error "First sorted list item wrong."))
    (if (neq (get theList 1) "head") (error "Second sorted list item wrong."))
    (if (neq (get theList 2) "leg") (error "Third sorted list item wrong."))
    (if (neq (get theList 3) "tail") (error "Fourth sorted list item wrong."))
}
function testListBuilder(: theList) {
    ("Testing list builder expression...[br]")
    (set theList (list 1 2 3))
    (if (neq (size theList) 3) (error "Created list has wrong size."))
    (if (neq (get theList 0) 1) (error "Created list has wrong first value"))
    (if (neq (get theList 1) 2) (error "Created list has wrong second value"))
    (if (neq (get theList 2) 3) (error "Created list has wrong third value"))
}

// ////////////////////////////////////////////////////////////////////////////
// Test Map Methods
// ////////////////////////////////////////////////////////////////////////////
asm_function testMaps(: keys) {
    "\n# Testing Map Operations\n" say

    "Fetching map keys...[br]" say
    7 testMap get 0 eq test_valid_key jnz
    "Accessing invalid element returns wrong value." error

    test_valid_key:
    2 testMap get "End" eq test_update jnz
    "Retrieving map key \"2\" returned invalid result." error

    test_update:
    "Testing key update...[br]" say
    "Apple" 3 testMap setp
    3 testMap get "Apple" eq test_newkey jnz
    "Retrieving updated map key \"3\" returned invalid result." error

    test_newkey:
    "Testing inserting new key...[br]" say
    "item" "newkey" testMap setp
    "newkey" testMap get "item" eq test_haskey jnz
    "Retrieving new map key returned invalid result." error

    test_haskey:
    "Testing has_key...[br]" say
    2 testMap has test_haskey_2 jnz
    "Map denies having key \"2\"." error

    test_haskey_2:
    7 testMap has test_delete jz
    "Map claims to have key \"7\"." error

    test_delete:
    "newkey" testMap del
    "newkey" testMap get 0 eq test_key_list jnz
    "Failed to delete newkey." error

    test_key_list:
    "Converting testMap keys to list...[br]" say
    testMap get_keys *keys set
    0 keys get 1 eq found_bad_key jz
    1 keys get 2 eq found_bad_key jz
    2 keys get 3 eq found_bad_key jz
    3 keys get 4 eq found_bad_key jz
    keys size 4 eq all_done jnz
    "Map keys list is wrong size." error

    all_done:
    return
    found_bad_key:
    "Key list has invalid value." error
}


// ////////////////////////////////////////////////////////////////////////////
// Test Dynamic Creation
// ////////////////////////////////////////////////////////////////////////////
asm_function testDynamic(: newObj newStr newList newMap) {
    "\n#Testing Dynamic Creation\n" say

    "Creating data...[br]" say
    String new *newStr set
    newStr typeof String eq makeNewObject jnz
    "New string not of String type." error

    makeNewObject:
    Object new *newObj set
    newObj typeof Object eq makeNewList jnz
    "New object not of Object type." error

    makeNewList:
    List new *newList set
    newList typeof List eq makeNewMap jnz
    "New list not of List type." error

    makeNewMap:
    Map new *newMap set
    newMap typeof Map eq testNewObjectProperties jnz
    "New map not of Map type." error


    testNewObjectProperties:
    "Testing property manipulation of created object...[br]" say
    $anInt newObj get 0 eq testObjectUnsetHas jnz
    "New object returns value for unset property." error

    testObjectUnsetHas:
    $anInt newObj has testObjectSet jz
    "New object claims to have anInt property." error

    testObjectSet:
    66 $anInt newObj setp
    $anInt newObj get 66 eq testObjectSetHas jnz
    "Updated property on new object did not retain value." error

    testObjectSetHas:
    $anInt newObj has testMapValues jnz
    "New object denies having anInt property." error


    testMapValues:
    "Testing values of new map...[br]" say
    "key" newMap get 0 eq testSetMapValue jnz
    "New map returns value for unset key...[br]" error

    testSetMapValue:
    "New Map Value" "key" newMap setp
    "key" newMap get "New Map Value" eq testListValues jnz
    "New map returns 0 for updated key...[br]" error


    testListValues:
    "Testing values of new list...[br]" say
    newList size 0 eq testListAddValue jnz
    "New list is not empty." error

    testListAddValue:
    "New List Item" 0 newList ins
    0 newList get "New List Item" eq testIsStatic jnz
    "List item not updated." error

    testIsStatic:
    "Testing isStatic...[br]" say
    testDynamic is_static reportedStaticAsDynamic jz
    "Fred" is_static reportedStaticAsDynamic jz
    testList is_static reportedStaticAsDynamic jz

    newObj is_static reportedDynamicAsStatic jnz
    newStr is_static reportedDynamicAsStatic jnz
    return

    reportedStaticAsDynamic:
    "Reported static value as dynamic." error

    reportedDynamicAsStatic:
    "Reported dynamic value as static." error

    List new List new List new say
}

// ////////////////////////////////////////////////////////////////////////////
// Test String Manipulation
// ////////////////////////////////////////////////////////////////////////////
asm_function testStringManipulation(: testString) {
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
    "Fred" testString str_compare stringAppend jz
    "Dynamic and static strings did not compare equal." error

    stringAppend:
    "Appending to test string...[br]" say
    testString str_clear
    "fred" testString str_append
    " Lives!" testString str_append
    "fred Lives!" testString str_compare stringAppendUF jz
    "String append malfunctioned." error

    stringAppendUF:
    "Appending to test string with initial upper...[br]" say
    testString str_clear
    "abc" testString str_append_uf
    "Abc" testString str_compare doneStrings jz
    "String append UF malfunctioned." error

    doneStrings:
    0 testStringBuilder call
}

function testStringBuilder(: theString) {
    ("Testing string builder...[br]")
    (set theString (string "I ate " 3 " turnips."))
    (if (str_compare theString "I ate 3 turnips.")
        (error "newstr did not create string correctly."))
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

// ////////////////////////////////////////////////////////////////////////////
// All tests runner
// ////////////////////////////////////////////////////////////////////////////
asm_function main() {
    "GTRPGE Test Suite" infobarTitle set_setting
    "GTRPGE Test Suite" infobarLeft set_setting

    0 testJumps call
    0 testCompare call
    0 testCall call
    0 testStack call
    0 testMath call
    0 testDefaults call

    0 testValues call
    0 testLists call
    0 testMaps call
    0 testObjectProps call
    0 testDynamic call
    0 testStringManipulation call
    0 testAndOr call

    "\nALL TESTS PASSED" say
}
