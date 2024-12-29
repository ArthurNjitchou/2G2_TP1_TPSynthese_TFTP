#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUFSIZE 1024   // Buffer size
#define PORT "1069"    // Port on the server
#define MAXDATA 512    // Maximum packet size

int main(int argc, char *argv[]) {
    // Variable declarations
    struct addrinfo hints;
    struct addrinfo *serverInfo;
    struct addrinfo *cur;
    int so;
    socklen_t slen;
    int sockfd;
    int messageLength;
    char *buf;
    struct sockaddr_in addr;


    // Retrieve arguments
    char *server = argv[2];    // Server IP address
    char *file = argv[3];      // File name
    char *option = argv[4];    // Blocksize option
    char *blocksizeValue = argv[5];  // Blocksize value
    char *mode = "octet";  // Mode set to "octet"

    // UDP client configuration
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;  // Datagram mode (UDP)
    if ((so = getaddrinfo(server, PORT, &hints, &serverInfo)) != 0) {
        perror("Client: 'getaddrinfo' error\n");
        exit(EXIT_FAILURE);
    }

    // Socket creation
    for (cur = serverInfo; cur != NULL; cur = cur->ai_next) {
        if ((sockfd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol)) == -1) {
            perror("Client: 'socket' error\n");
            continue;
        }
        break;
    }

    if (cur == NULL) {
        fprintf(stderr, "Client: Unable to create a socket\n");
        return 2;
    }

    

    return 0;
}

