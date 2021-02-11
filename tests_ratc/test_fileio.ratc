declare TITLE   "Automated Test Suite for File IO";
declare AUTHOR  "Gren Drake";
declare VERSION 1;
declare GAMEID  "";

declare FILE_NAME 0;
declare TEST_FILE_NAME "Short Test File";

function delete_all_files() {
    [ theFilelist counter length ]
    (set theFilelist (file_list GAMEID))
    (set counter 0)
    (set length (size theFilelist))
    (while (lt counter length)
        (proc
            (file_delete (get (get theFilelist counter) FILE_NAME))
            (inc counter)))
    (return length)
}

function main() {
    [ theFilelist filedata aList ]
    ("\n# Testing file io...")

    (delete_all_files)
    (set theFilelist (file_list GAMEID))

    // make sure we're starting with zero files
    (if (neq (size theFilelist) 0) (error "File list is not empty before tests."))

    // write a short test file
    (set aList (new List))
    (list_push aList 4)
    (list_push aList 8)
    (list_push aList 20)
    (list_push aList 60)
    (file_write TEST_FILE_NAME aList)

    (set filedata (file_read TEST_FILE_NAME GAMEID))
    (if (eq filedata none) (error "Failed to load test file."))
    (if (neq (size filedata) 4) (error "File content is wrong size."))
    (if (neq (get filedata 0) 4)  (error "First file value is wrong."))
    (if (neq (get filedata 1) 8)  (error "Second file value is wrong."))
    (if (neq (get filedata 2) 20) (error "Third file value is wrong."))
    (if (neq (get filedata 3) 60) (error "Fourth file value is wrong."))

    // delete test file
    (if (not (file_delete TEST_FILE_NAME)) (error "Failed to delete test file."))

    // verify no files remain
    (if (neq (size theFilelist) 0) (error "File list is not empty after test."))

    // ensure deleting non-existant file fails
    (if (file_delete TEST_FILE_NAME) (error "Successfully deleted file that does not exist."))
}
