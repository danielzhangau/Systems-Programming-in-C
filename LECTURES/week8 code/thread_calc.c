#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* do_cube(void* v) {
    int val = *(int*)v;
    free(v);
    int* result = malloc(sizeof(int));
    *result = val * val * val;
    return (void*)result;
}

// In this limited case we could have used the same pointer
// for in and out
void* alt_cube(void* v) {
    int* p = (int*)v;
    int value = *p;
    *p = value * value * value;
    return (void*)0;
}


int main(int argc, char** argv) {
    pthread_t tid;
    int* p = malloc(sizeof(int));
    *p = 4;
    pthread_create(&tid, 0, do_cube, p); 
        // Now we wait for the thread to finish
	// We need somewhere to store the return value
    void* res;
    pthread_join(tid, &res);	// &res is void**
    printf("Thread returned %d\n", *(int*)res);
    return 0;
}
