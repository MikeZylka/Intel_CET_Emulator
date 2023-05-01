#include <stdio.h>
#include <string.h>

void func1(void);
void func2(void);

int main(int argc, char *argv[]) {
    void (*func_ptr)(void);
    char buffer[20];
    memset(buffer, 0, sizeof(buffer));
    func_ptr = func1;
    printf("function pointer = %p\n", func_ptr);
    printf("Enter a string: ");
    gets(buffer);
    func_ptr();
    printf("Program terminated normally.\n");
    return 0;
}

void func1(void) {
    printf("Hello from function 1!\n");
}

void func2(void) {
    printf("Hello from function 2!\n");
}