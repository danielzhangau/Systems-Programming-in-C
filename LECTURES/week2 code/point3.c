#include <stdio.h>
#include <stdlib.h>

void func(void) {
    int* p = (int*)malloc(sizeof(int));	// bad variable name
    printf("%p %d\n", p, *p);	// will generate warnings
    *p = 34;
    printf("%p %d\n", p, *p);	// will generate warnings
    p = (int*)malloc(sizeof(int));
    printf("%p %d\n", p, *p);	// will generate warnings
}

int main(int argc, char** argv) {
    func();
    return 0;
}
