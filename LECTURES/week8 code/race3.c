#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#define LOOPS 200000
#define PCOUNT 6

struct Param {
    int* value;
    pthread_t tid;
    sem_t* guard;
};

void init_lock(sem_t* l) {
    sem_init(l, 0, 1);
}

void  take_lock(sem_t* l) {
    sem_wait(l);
}

void release_lock(sem_t* l) {
    sem_post(l);
}

void* fn(void* v) {
    struct Param* p = (struct Param*)v;
    p->tid=pthread_self();
    for (int i=0; i < LOOPS; ++i) {
	take_lock(p->guard);
        (*(p->value))++;
	release_lock(p->guard);
    }
    return 0;
}


int main(int argc, char** argv) {
    int total = 0;
    sem_t l;
    init_lock(&l);
    struct Param pars[PCOUNT];
    pthread_t tids[PCOUNT];
    for (int i = 0; i < 6; ++i) {
	pars[i].value = &total;
        pars[i].guard = &l;
        pthread_create(&(tids[i]), 0, fn, pars+i);
    }
    for (int i = 0;i < 6; ++i) {
	void* v;
        pthread_join(tids[i], &v);
    }
    sem_destroy(&l);
    printf("Expected %d got %d\n", LOOPS*PCOUNT, total);
    printf("Lost %d updates\n", LOOPS*PCOUNT-total);
    return 0;
}
