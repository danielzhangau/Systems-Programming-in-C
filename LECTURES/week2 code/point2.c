#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int* p = (int*)malloc(sizeof(int));	// bad variable name
    printf("%p %d\n", p, *p);	// will generate warnings
    *p = 34;
    printf("%p %d\n", p, *p);	// will generate warnings
    p = (int*)malloc(sizeof(int));
    printf("%p %d\n", p, *p);	// will generate warnings
    return 0;
}
