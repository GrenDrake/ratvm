declare TITLE   "Automated Test Suite for Lists";
declare AUTHOR  "Gren Drake";
declare VERSION 1;
declare GAMEID  "";

declare testList [ 1 2 3 ];
extend testList [ 100 200 ];
object testObject;

// ////////////////////////////////////////////////////////////////////////////
// Test List Methods
// ////////////////////////////////////////////////////////////////////////////
function printArray(theArray) {
    [ counter length ]
    ("[b]Array contents:[/b]")
    (set length (size theArray))
    (set counter 0)
    (while (lt counter length)
        (proc
            (" " (get theArray counter))
            (inc counter)))
    (" (length: " length ")\n")
}

function main() {
    [ counter random_value ]
    (asm
        "\n# Testing List Operations\n" say

        "Testing list insertion...[br]" say
        testList size 5 eq test_index_of jnz
        "Test list has wrong initial size." error

        test_index_of:
        "Testing index_of...[br]" say
        testList 1 indexof 0 eq test_indexof_bad1 jz
        testList 2 indexof 1 eq test_indexof_bad2 jz
        testList 3 indexof 2 eq test_indexof_bad3 jz
        testList 4 indexof 0 gt test_indexof_bad4 jz

        test_add jmp
        test_indexof_bad1: "Wrong result for position of \"1\" in test list." error
        test_indexof_bad2: "Wrong result for position of \"2\" in test list." error
        test_indexof_bad3: "Wrong result for position of \"3\" in test list." error
        test_indexof_bad4: "Wrong result for position of \"4\" in test list." error

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
        testObject 2 testList setp
        2 testList get testObject eq test_deletion jnz
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
        ret

        test_list_contents_wrong:
        "\n" say testList 1 printArray call
        "List has wrong contents." error
        ret
    )
}

function testListSort() {
    [ theList ]
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

function testListBuilder() {
    [ theList ]
    ("Testing list builder expression...[br]")
    (set theList (list 1 2 3))
    (if (neq (size theList) 3) (error "Created list has wrong size."))
    (if (neq (get theList 0) 1) (error "Created list has wrong first value"))
    (if (neq (get theList 1) 2) (error "Created list has wrong second value"))
    (if (neq (get theList 2) 3) (error "Created list has wrong third value"))
}
