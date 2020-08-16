#include <pthread.h>
#include <stdio.h>

#define LOOPS 200000
#define PCOUNT 6

struct Lock {
    pthread_t who;
};

struct Param {
    int* value;
    pthread_t tid;
    struct Lock* guard;
};

void init_lock(struct Lock* l) {
    l->who = 0;
}

void  take_lock(struct Lock* l) {
    while (l->who != 0) {}
    l->who = pthread_self();
}

void release_lock(struct Lock* l) {
    l->who = 0;
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
    struct Lock l;
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
    printf("Expected %d got %d\n", LOOPS*PCOUNT, total);
    printf("Lost %d updates\n", LOOPS*PCOUNT-total);
    return 0;
}
