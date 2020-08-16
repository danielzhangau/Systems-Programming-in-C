#include <stdio.h>

int main(int argc, char** argv) {
    int total=0;
    for (int i = 1; i < argc; ++i) {
        total+= (int)argv[i];
    }
    printf("%d\n", total);
    return 0;
}
