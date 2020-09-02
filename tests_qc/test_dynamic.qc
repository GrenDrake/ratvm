
declare TITLE   "Automated Test Suite for Dynamic Value Creation and Destruction";
declare AUTHOR  "Gren Drake";
declare VERSION 1;
declare GAMEID  "";


declare testList [ 1 2 3 ];


// ////////////////////////////////////////////////////////////////////////////
// Test Dynamic Creation
// ////////////////////////////////////////////////////////////////////////////
function testDynamic() {
    [ newObj newStr newList newMap ]
    (asm
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
        main is_static reportedStaticAsDynamic jz
        "Fred" is_static reportedStaticAsDynamic jz
        testList is_static reportedStaticAsDynamic jz

        newObj is_static reportedDynamicAsStatic jnz
        newStr is_static reportedDynamicAsStatic jnz
        ret

        reportedStaticAsDynamic:
        "Reported static value as dynamic." error

        reportedDynamicAsStatic:
        "Reported dynamic value as static." error

        List new List new List new say
    )
}

function testGarbage(localVar fakeRef) {
    (asm
        "\n# Testing garbage collection\n" say

        // we convert the new list to an int to ensure there are no references to it
        // this will permit the garbage collector to collect the list
        Integer List new astype *fakeRef set
        List new *localVar set
        List new // leave value on top of stack

        // force garbage collection
        "Objects Collected: " say collect say "[br]" say

        "Testing uncollected objects...[br]" say
        // the new list from earlier is still on the top of the stack
        is_valid true eq stack_not_valid jz
        localVar is_valid true eq localvar_not_valid jz

        "Testing collected objects...[br]" say
        List fakeRef astype is_valid false eq list_still_valid jz

        ret
        list_still_valid:   "Unreferenced list not collected." error
        stack_not_valid:    "Stack item collected." error
        localvar_not_valid: "Local variable item collected." error
    )
}

function main() {
    (testDynamic)
    (testGarbage)
}
