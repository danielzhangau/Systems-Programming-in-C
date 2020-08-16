#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

int base=0;

void* doer(void* p) {
    int x=*(int*)p;
    while (1) {
        sleep(2);
	printf("%d\n", x+base);
    }
    return 0;
}


int main(int argc, char** argv) {
    bool parent = true;
    pthread_t t;
    int values[4] = {1,2,3,4};
    pthread_create(&t, 0, doer,(void*)(values+0));
    pthread_create(&t, 0, doer,(void*)(values+1));
    pthread_create(&t, 0, doer,(void*)(values+2));
    pthread_create(&t, 0, doer,(void*)(values+3));

    if (!fork()) {
        base = 10;
	parent=false;
    } else {
        base = 200;
    }
    while (1) {
        sleep(1);
	printf("Main-%s\n", (parent)?"parent":"child");
    }
    return 0;
}
