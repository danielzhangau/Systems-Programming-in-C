#include <pthread.h>
#include <stdio.h>

#define LOOPS 200000
#define PCOUNT 6

void* fn(void* v) {
    int* p=(int*)v;
    for (int i=0; i < LOOPS; ++i) {
        (*p)++;
    }
    return 0;
}


int main(int argc, char** argv) {
    int total = 0;
    pthread_t tids[PCOUNT];
    for (int i = 0; i < 6; ++i) {
        pthread_create(&(tids[i]), 0, fn, &total);
    }
    for (int i = 0;i < 6; ++i) {
	void* v;
        pthread_join(tids[i], &v);
    }
    printf("Expected %d got %d\n", LOOPS*PCOUNT, total);
    printf("Lost %d updates\n", LOOPS*PCOUNT-total);
    return 0;
}
