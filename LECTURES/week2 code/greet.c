#include <string.h>
#include <stdio.h>

int main(int argc, char** argv) {
    char buffer[6];
    strcpy(buffer, "Hello");
    char* greet=buffer;
    printf("%s %s\n", greet, buffer);
    greet=&(buffer[2]);
    printf("%s %s\n", greet, buffer);
    return 0;
}
