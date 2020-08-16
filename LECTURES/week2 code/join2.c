#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int size = 0;
    for (int i = 1; i < argc; ++i) {
	size += strlen(argv[i]);
    }
    size += 1;	// need space to store \0
    char* buffer=(char*)malloc(sizeof(char)*size);
    buffer[0]='\0';  // equivalent to strcpy(buffer, "") but faster
    for (int i=1; i<argc; ++i) {
        strcat(buffer, argv[i]);	// concatenate
    }
    printf("%s\n", buffer);
    free(buffer);
    return 0;
}
