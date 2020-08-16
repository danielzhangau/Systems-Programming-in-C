#include <stdio.h>
#include <stdlib.h>

int* func(void) {
    int* p = (int*)malloc(sizeof(int));	// bad variable name
    printf("%p %d\n", p, *p);	// will generate warnings
    *p = 34;
    printf("%p %d\n", p, *p);	// will generate warnings
    free(p);
    p = (int*)malloc(sizeof(int));
    printf("%p %d\n", p, *p);	// will generate warnings
    return p;
}

int main(int argc, char** argv) {
    int* p2 = func();
    int* p3 = p2;
    free(p2);
    printf("%p %d\n", p3, *p3);	// will generate warnings
    return 0;
}
