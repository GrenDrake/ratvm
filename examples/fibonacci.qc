declare TITLE   "Fibonacci Benchmark";
declare AUTHOR  "Gren Drake";
declare VERSION 1;
declare GAMEID  "5AF56A1B-3BBF-47A4-B986-7BC7C59FB401";


function fibonacci( n ) {
    (if (lte n 1)
        (return n))

    (return
        (add
            (fibonacci (sub n 2))
            (fibonacci (sub n 1))))
}

function main() {
    ("Fibonacci(34): ")
    (print (fibonacci 34)) // = 5702887 (5,702,887)
    ("\n")
}