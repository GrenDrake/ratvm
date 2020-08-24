/*
    This is a small, simple Fibonacci calculator used for benchmark comparison
    tests.

    This program is included as part of QuollVM and made available under the
    terms of the MIT license <https://opensource.org/licenses/MIT>
*/

#include <stdio.h>

int fibonacci(int n) {
    if (n < 2) return n;
    return (fibonacci(n-2) + fibonacci(n-1));
}

int main() {
    printf("expected: 5702887\n  result: %d\n", fibonacci(34));
    return 0;
}