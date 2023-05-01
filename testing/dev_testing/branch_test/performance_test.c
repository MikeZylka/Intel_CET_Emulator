#include <stdio.h>
#include <limits.h>

unsigned long foo_count = 0;
unsigned long bar_count = 0;

void foo() {
    // Increment the global foo count variable
    foo_count++;
}

void bar() {
    // Increment the global bar count variable
    bar_count++;
}

int main() {
    for (unsigned long i = 0; i < 1000000000; i++) {
        foo();
        bar();
    }

    // Print the final counts for foo() and bar()
    printf("foo_count=%lu, bar_count=%lu\n", foo_count, bar_count);

    return 0;
}