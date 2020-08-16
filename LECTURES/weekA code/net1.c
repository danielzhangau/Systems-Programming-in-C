#include <netdb.h>
#include <string.h>
#include <stdio.h>

/* Summarised from /usr/include/netinet/in.h


struct sockaddr_in {
...
   in_port_t sin_port;
   struct in_addr sin_addr;
...
};

struct in_addr {
    in_addr_t s_addr;
};

*/


// Print dotted byte form of the given IP address
void print_quads(in_addr_t x) {
    unsigned char* bytes = (unsigned char*)&x;  
    for (int i=0; i<sizeof(x); ++i) {
        printf("%u.", (unsigned)bytes[i]);
    }
}

// argv[1] is a port number but (apart from it !=0) it doesn't
// matter what it is for this example
int main(int argc, char** argv) {
    if (argc<2) {
        fprintf(stderr, "Not enough args\n");
        return 2;
    }
    const char* port=argv[1];
    struct addrinfo* ai = 0;	// Will point to the answer to our addr query
    struct addrinfo hints;	// This holds hints about what we want
    memset(& hints, 0, sizeof(struct addrinfo));
    hints.ai_family=AF_INET;        // IPv4  for generic could use AF_UNSPEC
    hints.ai_socktype=SOCK_STREAM;  // We want TCP
    int err;
    if (err=getaddrinfo("moss.labs.eait.uq.edu.au", port, &hints, &ai)) {
        freeaddrinfo(ai);
        fprintf(stderr, "%s\n", gai_strerror(err));
        return 1;   // could not work out the address
    }
    // now let's try to get the IP address out
    // See man getaddrinfo for the addrinfo struct
    struct sockaddr* sa=ai->ai_addr;

    // There is more than one type of socket address
    // So cast to the one we _know_ was sent back
    struct sockaddr_in* sai=(struct sockaddr_in*)sa;


    in_addr_t address = sai->sin_addr.s_addr;

    print_quads(address);   // does not consider endianness
                            // inet_ntoa() would be better
    return 0;
}
