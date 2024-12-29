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

    // Code for gettftp (downloading file)
    if (strcmp(argv[1], "gettftp") == 0) {
        // ------ Construct a Read Request (RRQ) and send to the server ------
        
        // Building the RRQ (Read Request)
        char *RRQ;
        int len = strlen(file) + strlen(mode) + 4;
        if (argc == 6) {
            if (strcmp(option, "-blocksize") == 0) {
                len = strlen(file) + strlen(mode) + 4 + 2 + strlen("blksize") + strlen(blocksizeValue);
            }
        }

        RRQ = malloc(len);
        RRQ[0] = 0;
        RRQ[1] = 1; // Opcode for RRQ
        strcpy(RRQ + 2, file);
        RRQ[2 + strlen(file)] = 0;
        strcpy(RRQ + 3 + strlen(file), mode);
        RRQ[3 + strlen(file) + strlen(mode)] = 0;

        // Handle the blocksize option if provided
        if (argc == 6) {
            if (strcmp(option, "-blocksize") == 0) {
                strcpy(RRQ + 4 + strlen(file), "blksize");
                RRQ[4 + strlen(file) + strlen("blksize")] = 0;
                strcpy(RRQ + 5 + strlen(file) + strlen("blksize"), blocksizeValue);
                RRQ[5 + strlen(file) + strlen("blksize") + strlen(blocksizeValue)] = 0;
            }
        }

        // Send the RRQ to the server
        if ((messageLength = sendto(sockfd, RRQ, len, 0, (struct sockaddr *) cur->ai_addr, cur->ai_addrlen)) == -1) {
            perror("Client: 'sendto RRQ' error\n");
            exit(EXIT_FAILURE);
        }
        printf("CLIENT: Sent RRQ of %d bytes to the server\n", messageLength);

        // ------ Receive the file -------
        int dataReceived;
        FILE *downloadedFile = fopen(file, "wb");  // Open file to save downloaded content
        do {
            buf = malloc(BUFSIZE);
            if ((messageLength = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *) &addr, &slen)) == -1) {
                perror("Client: 'recvfrom' error\n");
                exit(EXIT_FAILURE);
            }
            printf("CLIENT: Received packet number %d of %d bytes\n", *(buf + 3), messageLength - 4);
            buf[messageLength] = '\0';

            // Write the received data to the file
            dataReceived = messageLength - 4;
            fwrite(buf + 4, sizeof(char), dataReceived, downloadedFile);

            // ------ Acknowledge the received packet -------
            u_int16_t *ACK;
            len = 4;
            ACK = malloc(len);
            *ACK = htons((u_int16_t) 4);  // Opcode for ACK
            *(ACK + 1) = htons((u_int16_t) *(buf + 3));  // Block number

            // Send the ACK
            if ((sendto(sockfd, ACK, len, 0, (struct sockaddr *) &addr, slen)) == -1) {
                perror("Client: 'sendto ACK' error\n");
                exit(EXIT_FAILURE);
            }
            printf("CLIENT: Acknowledgment sent for packet number %d\n", *(buf + 3));
        } while (dataReceived == MAXDATA);  // Continue while dataReceived = 512 bytes

        fclose(downloadedFile);
        printf("CLIENT: File transfer complete\n");
    }

    

    return 0;
}

