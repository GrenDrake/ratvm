declare TITLE   "Interactive Test Suite";
declare AUTHOR  "Gren Drake";
declare VERSION 1;
declare GAMEID  "56735BDD-C5EE-4837-911D-BBE70676C89A";


// ////////////////////////////////////////////////////////////////////////////
// Data used for tests
// ////////////////////////////////////////////////////////////////////////////
declare testInt 7384;
declare testStr "A Name";
declare testList [ 1 2 3 ];
declare testMap { 1: "The" 2: "End" 3: "Is" 4: "Far" };
declare testProperty $testFunc;
declare testFlags flags( 6 7 8 testInt );

object testObject
    $testMethod function() { }
;

// ////////////////////////////////////////////////////////////////////////////
// Code for testing load/save
// ////////////////////////////////////////////////////////////////////////////

function printMagicNumbers() {
    ("\nThis morning's magic numbers are: "
        (get magic_1 $number1) ", "
        (get magic_1 $number2) ", "
        (get magic_1 $number3))
    (". This evening's magic numbers are: "
        (get magic_2 $number1) ", "
        (get magic_2 $number2) ", "
        (get magic_2 $number3) ".\n")
}

object magic_1
    $ident 1
    $save asm_function(data) {
        $number1 self get 99 data ins
        $number2 self get 99 data ins
        $number3 self get 99 data ins
    }
    $load asm_function(data) {
        0 data get $number1 self set
        1 data get $number2 self set
        2 data get $number3 self set
    }
    $number1 0
    $number2 0
    $number3 0
;

object magic_2
    $ident 2
    $save asm_function(data) {
        $number1 self get 99 data ins
        $number2 self get 99 data ins
        $number3 self get 99 data ins
    }
    $load asm_function(data) {
        0 data get $number1 self set
        1 data get $number2 self set
        2 data get $number3 self set
    }
    $number1 0
    $number2 0
    $number3 0
;


// ////////////////////////////////////////////////////////////////////////////
// Testing non-asm syntax constructs
// ////////////////////////////////////////////////////////////////////////////
function testSyntax(: value) {
    (print_uf "this " "tests " "print_uf.[br]")

    ("Testing IF true: ")
    (set value 5)
    (if value ("Good[br]") (error "IF true failed"))
    ("Testing IF false: ")
    (if 0 (error "IF false failed") ("Good[br]"))
    ("Testing IF false with no else[br]")
    (if 0 (error "IF false failed"))

    ("Testing proc statement:")
    (if 1
        (proc
            (" first")
            (" second")
            (" third"))
        (error "Reached unreachable else"))
    ("[br]")

    ("Testing while:")
    (set value 20)
    (while (gt value 0)
        (proc
            (" " value)
            (dec value 2)))
    ("[br]")

    ("Testing while with false initial.[br]")
    (set value 1)
    (while 0 (error "While loop with false condition executed."))

    ("Testing continue / break:")
    (set value 0)
    (while 1
        (proc
            (if (gt value 50) (break))
            (inc value)
            (if (neq (mod value 5) 0) (continue))
            (" " value)))
    ("[br]")

    ("Testing do/while:")
    (set value 1)
    (do_while
        (proc
            (" " value)
            (inc value 3))
        (lte value 20))
    ("[br]")

    ("Testing do/while with false condition: ")
    (do_while
        ("in loop")
        0)
    ("[br]")

    ("Testing continue / break in do/while:")
    (set value 55)
    (do_while
        (proc
            (if (lte value 0) (break))
            (dec value)
            (if (neq (mod value 5) 0) (continue))
            (" " value))
        1)
}


// ////////////////////////////////////////////////////////////////////////////
// Basic looping demo
// ////////////////////////////////////////////////////////////////////////////
function testLoopDemo(: counter theValue) {
    (set counter 0)
    (set theValue 1)
    ("Counting:")
    (do_while
        (proc
            (inc counter)
            (set theValue (mult theValue 2))
            (" " counter))
        (lt counter 10))
    (". Loop finished!\nFinal result: " theValue "\n")
}


// ////////////////////////////////////////////////////////////////////////////
// Test input commands
// ////////////////////////////////////////////////////////////////////////////
function testInputs(: choice xtra) {
    (set choice 1)
    (while 1 (proc
        ("This node serves as a test of the menu selection options. Other input options can be selected in the menu below, or you can return to the top level menu.")
        (option "Test key input"        testGetkey)
        (option "Test line input"       testLineInput)
        (option "No extra value"        receiveExtra)
        (option "Extra value 6"         receiveExtra   6)
        (option "Extra value 73"        receiveExtra   73)
        (option "Value 2 (key: G)"      receiveExtra   2    'g')
        (option "Return to main menu"   none)
        (set choice (get_option xtra))
        (if choice
            (dispatcher choice xtra)
            (return 0))))
}
function receiveExtra(extra) {
    ("Option extra value: " extra)
    (option "Back to inputs menu" testInputs)
    (get_option none)
}
function testGetkey(: inKey) {
    ("This will demonstrate and test the get_key input type. Press 'Z' to return to the previous menu.")
    (while 1 (proc
        (set inKey (get_key "Try pressing a key."))
        ("Resulting character code: " inKey ".\n")
        (if (eq inKey 'z') (return 0)))
}
object lineInputHolder
    $lastLine "(nothing yet)"
;
function testLineInput(: theLine) {
    ("Testing line input option.\nThe last line you entered was: ")
    (print (get lineInputHolder $lastLine) "\n")
    (set theLine (get_line "Enter some text:"))
    ("You entered: \"" theLine "\".\n")
    (setp lineInputHolder $lastLine theLine)
    (option "Back to inputs menu" testInputs)
    (get_option none)
}


// ////////////////////////////////////////////////////////////////////////////
// Output command tests
// ////////////////////////////////////////////////////////////////////////////
function testOutput() {
    ("This text (and nearly all text in these tests) is output using the \"print\" expression.\n")
    ("-1 using say_unsigned (should be 4294967295): ")
    (say_unsigned -1)
    ("[br] using say_char (should be 'A'): ")
    (say_char 65)
    ("[br]Say UTF-8 Text: テキストの例\n")
    ("Horizontal rule below this:\n[hr]\nText below horizontal rule.\n")

    ("# Testing text formatting:\n")
    ("bold text: [b]Lorem ipsum dolor sit amet.[/b][br]")
    ("italic text: [i]Lorem ipsum dolor sit amet.[/i][br]")
    ("bold italic: [b][i]Lorem ipsum dolor sit amet.[/i][/b][br]")
    ("italic bold: [i][b]Lorem ipsum dolor sit amet.[/b][/i][br]")
    ("bold italic with different ranges: [b]Lorem [i]ipsum dolor sit[/i] amet.[/b]\n")
    ("Testing coloured text:
        [color red]RED[/color]
        [color green]GREEN[/color]
        [color yellow]YELLOW[/color]
        [color blue]BLUE[/color]
        [color magenta]MAGENTA[/color]
        [color cyan]CYAN[/color]
        [color default]DEFAULT[/color]\n")

    ("Testing multiline string. Linebreak occurs here -->

    <--\n")

    ("Testing output of non-string types:[br][b]Integer[/b]: " 55)
    ("[br][b]List[/b]: " testList)
    ("[br][b]Map[/b]: " testMap)
    ("[br][b]Object[/b]: " testObject)
    ("[br][b]Property[/b]: " testProperty "\n")
}

// ////////////////////////////////////////////////////////////////////////////
// Update info areas
// ////////////////////////////////////////////////////////////////////////////
function testInfoarea() {
    ("The header, footer, and title regions should now be updated. (Unless you already ran this test since starting the program.)")

    (set_setting infobarLeft   "Good day!")
    (set_setting infobarRight  "5:00 am")
    (set_setting infobarFooter "Fred is Cool")
    (set_setting infobarTitle  "Updated Titled")
}

// ////////////////////////////////////////////////////////////////////////////
// Test RNG
// ////////////////////////////////////////////////////////////////////////////
function testRandom(: counter) {
    ("This will generate 200 random numbers between 50 and 60 below.\n")

    (set counter 200)
    (while (gt counter 0)
        (proc
            (print (random 50 60) " ")
            (dec counter)
            (if (eq (mod counter 20) 0)
                ("[br]")
                (if (eq (mod counter 10) 0)
                    (" : ")))))
}

// ////////////////////////////////////////////////////////////////////////////
// Test Fetching Debug / Origin Info
// ////////////////////////////////////////////////////////////////////////////
function testOrigins() {
    ("[b]Origin of values with origin:[/b][br]")
    ("main: "         (origin main) "[br]")
    ("testList: "     (origin testList) "[br]")
    ("testMap: "      (origin testMap) "[br]")
    ("testObject: "   (origin testObject) "[br]")
    ("testObject.testMethod"
        (origin (get testObject $testMethod))
        "\n")

    ("[b]Origin of values without origin:[/b][br]")
    ("testInt: "      (origin testInt) "[br]")
    ("testStr: "      (origin testStr) "[br]")
    ("testProperty: " (origin testProperty) "[br]")
    ("testFlags: "    (origin testFlags) "\n")

    ("[b]Origin of dynamic values:[/b][br]")
    ("new List: "     (origin (new List)) "[br]")
}


// ////////////////////////////////////////////////////////////////////////////
// Test Page Commands
// ////////////////////////////////////////////////////////////////////////////
asm_function testPages() {
    "Demo Page" testPage1 'd' add_page
    "Another Page" otherPage 'a' add_page
    "Page" otherPage 'p' add_page
    "A Long Page Name" otherPage 'l' add_page
    0 testPagesReal call
}
asm_function testPagesReal() {
    "Extra pages have been added and can be activated by hotkey or by clicking on their button in the lower left." say
    "Remove \"Another Page\"" testPagesRemovePage none none add_option
    "Cleanup and return to main list" testPagesCleanup none none add_option
    none get_option
}
asm_function testPagesRemovePage() {
    "Another Page" del_page
    0 testPagesReal call
}
asm_function testPagesCleanup() {
    "Demo Page" del_page
    "Another Page" del_page
    "Page" del_page
    "A Long Page Name" del_page
    0 showOptions call
}

object testpageData
    $count 1
    $index 0
;
asm_function testPage1(key) {
    key typeof Integer eq showpage jz
    key 32 eq showkey jz
    end_page
    ret

    leftkey:
    $index testpageData get 1 sub $index testpageData setp
    showpage jmp
    rightkey:
    $index testpageData get 1 add $index testpageData setp
    showpage jmp
    upkey:
    $index testpageData get 10 mult $index testpageData setp
    showpage jmp
    downkey:
    $index testpageData get 10 div $index testpageData setp
    showpage jmp

    showkey:
    "You pressed: " say key say " (which is not space).\n" say
    key -1 eq leftkey jnz
    key -2 eq rightkey jnz
    key -3 eq downkey jnz
    key -4 eq upkey jnz
    showpage:
    "You have viewed this page " say
    $count testpageData get say
    " times. The index counter is " say
    $index testpageData get say
    ". Use arrow keys to change."
    "Press space to return." testPage1 get_key
    $count testpageData get 1 add $count testpageData setp
}
asm_function otherPage() {
    "You selected a page other than the demo page! (Only the demo page has actual content.)" say
    "Continue" none none none add_option
    endOtherPage none get_option
}
asm_function endOtherPage() {
    end_page
}


// ////////////////////////////////////////////////////////////////////////////
// Test selection
// ////////////////////////////////////////////////////////////////////////////
function showOptions(: choice) {
    (while 1 (proc
        ("Welcome to the feature test demo!\nSelect one of the options displayed below by entering its associated number in order to run the tests. When that is complete, you will be returned to this screen. The magic numbers below will are randomized every time you start the game, but will be maintained across saving and loading.")
        (printMagicNumbers)

        (option "Loop demo"                 testLoopDemo)

        (option "Test syntax constructs"    testSyntax)

        (option "Test output"               testOutput)
        (option "Test info areas"           testInfoarea)
        (option "Test random"               testRandom)
        (option "Test origins"              testOrigins)

        (option "Test pages"                testPages)
        (option "Test input methods"        testInputs)
        (set choice (get_option none))
        (dispatcher choice)
        ))
}


function dispatcher(optionFunc extraValue) {
    (optionFunc extraValue)

    (if (or
            (eq optionFunc showOptions)
            (eq optionFunc testPages)
            (eq optionFunc testPagesReal)
            (eq optionFunc testPagesRemovePage)
            (eq optionFunc receiveExtra)
            (eq optionFunc testGetkey)
            (eq optionFunc testLineInput)
            (eq optionFunc testInputs))
        (return))
    (option "Return to main page" showOptions)
    (get_option none)
}

function main() {
    (set_setting infobarTitle "GTRPGE Test Suite")
    (set_setting infobarLeft  "GTRPGE Test Suite")

    // set the "magic numbers" used for demonstrate the load/save code
    (setp magic_1 $number1 (random 10000 40000))
    (setp magic_1 $number2 (random 10000 40000))
    (setp magic_1 $number3 (random 10000 40000))
    (setp magic_2 $number1 (random 10000 40000))
    (setp magic_2 $number2 (random 10000 40000))
    (setp magic_2 $number3 (random 10000 40000))

    (showOptions)
}
