#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {
    struct addrinfo* ai = 0;
    struct addrinfo hints;
    memset(& hints, 0, sizeof(struct addrinfo));
    hints.ai_family=AF_INET;        // IPv4  for generic could use AF_UNSPEC
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE;  // Because we want to bind with it    
    int err;
    if (err=getaddrinfo("localhost", 0, &hints, &ai)) { // no particular port
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
    
        // Which port did we get?
    struct sockaddr_in ad;
    memset(&ad, 0, sizeof(struct sockaddr_in));
    socklen_t len=sizeof(struct sockaddr_in);
    if (getsockname(serv, (struct sockaddr*)&ad, &len)) {
        perror("sockname");
        return 4;
    }
    printf("%u\n", ntohs(ad.sin_port));
    
    if (listen(serv, 10)) {     // allow up to 10 connection requests to queue
        perror("Listen");
        return 4;
    }
    
    int conn_fd;
    char* msg="Go away!\n";
    while (conn_fd = accept(serv, 0, 0), conn_fd >= 0) {    // change 0, 0 to get info about other end
        FILE* stream = fdopen(conn_fd, "w");
        fputs(msg, stream);
        fflush(stream);
        fclose(stream);
    }
    return 0;
}
