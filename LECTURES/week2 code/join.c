#include <string.h>
#include <stdio.h>

int main(int argc, char** argv) {
    char buffer[100];	// not a great idea
    buffer[0]='\0';  	// equivalent to strcpy(buffer, "") but faster
    for (int i=1; i<argc; ++i) {
        strcat(buffer, argv[i]);	// concatenate
    }
    printf("%s\n", buffer);
    return 0;
}
