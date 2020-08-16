#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc<2) {
        fprintf(stderr, "Not enough args\n");
        return 2;
    }
    char* port = argv[1];
    struct addrinfo* ai = 0;
    struct addrinfo hints;
    memset(& hints, 0, sizeof(struct addrinfo));
    hints.ai_family=AF_INET;        // IPv4  for generic could use AF_UNSPEC
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE;  // Because we want to bind with it    
    int err;
    if (err=getaddrinfo("localhost", port, &hints, &ai)) { 
        freeaddrinfo(ai);
        fprintf(stderr, "%s\n", gai_strerror(err));
        return 1;   // could not work out the address
    }
    
        // create a socket and bind it to a port
    int serv = socket(AF_INET, SOCK_STREAM, 0); // 0 == use default protocol
    if (bind(serv, (struct sockaddr*)ai->ai_addr, sizeof(struct sockaddr))) {
        perror("Binding");
        return 3;
    }
    
    if (listen(serv, 10)) {     // allow up to 10 connection requests to queue
        perror("Listen");
        return 4;
    }
    
    int conn_fd;
    char* msg="Go away!\n";
    while (conn_fd = accept(serv, 0, 0), conn_fd >= 0) {
        FILE* stream = fdopen(conn_fd, "w");
        fputs(msg, stream);
        fflush(stream);
        fclose(stream);
    }
    return 0;
}
