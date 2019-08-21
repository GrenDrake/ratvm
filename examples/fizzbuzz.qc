
function main(: i) {
    (set i 1)
    (while (lte i 100)
        (proc
            (if (eq (mod i 15) 0)
                (print "FizzBuzz")
                (if (eq (mod i 3) 0)
                    (print "Fizz")
                    (if (eq (mod i 5) 0)
                        (print "Buzz")
                        (print i))))
            (print "[br]")
            (inc i)))
}
