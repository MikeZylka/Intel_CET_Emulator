#include <stdio.h>
#include <string.h>

void secretFunction() {
    printf("You have reached the secret function!\n");
}

void publicFunction() {
    printf("You have called the public function.\n");
}

void vulnerableFunction(char *str) {
    char buffer[10];
    strcpy(buffer, str);
    printf("You entered: %s\n", buffer);
}

int main() {
    char input[100];
    printf("Enter some text:\n");
    scanf("%s", input);
    vulnerableFunction(input);
    publicFunction();
    return 0;
}