declare TITLE   "Automated Test Suite for Maps";
declare AUTHOR  "Gren Drake";
declare VERSION 1;
declare GAMEID  "";

declare testMap {
    1: "The"
    2: "End"
};
extend testMap {
    3: "Is"
    4: "Far"
};

// ////////////////////////////////////////////////////////////////////////////
// Test Map Methods
// ////////////////////////////////////////////////////////////////////////////
function main() {
    [ keys ]
    (asm
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
        ret
        found_bad_key:
        "Key list has invalid value." error
    )
}