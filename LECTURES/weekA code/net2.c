#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void print_quads(in_addr_t x) {
    char* bytes = (char*)&x;  
    for (int i=0; i<sizeof(x); ++i) {
        printf("%u.", (unsigned)bytes[i]);
    }
}

int main(int argc, char** argv) {
    if (argc<2) {
        fprintf(stderr, "Not enough args\n");
        return 2;
    }
    const char* port=argv[1];
    struct addrinfo* ai = 0;
    struct addrinfo hints;
    memset(& hints, 0, sizeof(struct addrinfo));
    hints.ai_family=AF_INET;        // IPv4  for generic could use AF_UNSPEC
    hints.ai_socktype=SOCK_STREAM;
    int err;
    if (err=getaddrinfo("localhost", port, &hints, &ai)) {
        freeaddrinfo(ai);
        fprintf(stderr, "%s\n", gai_strerror(err));
        return 1;   // could not work out the address
    }

    int fd=socket(AF_INET, SOCK_STREAM, 0); // 0 == use default protocol
    if (connect(fd, (struct sockaddr*)ai->ai_addr, sizeof(struct sockaddr))) {
        perror("Connecting");
        return 3;
    }
    // fd is now connected
    // we want separate streams (which we can close independently)
    
    int fd2=dup(fd);
    FILE* to=fdopen(fd, "w");
    FILE* from=fdopen(fd2, "r");
    
    fprintf(to, "Hello\n");
    fflush(to);
    
    char buffer[80];
    fgets(buffer, 79, from);
    fprintf(stdout, "%s", buffer);
    return 0;
}
