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

    // Code for puttftp (uploading file)
    if (strcmp(argv[1], "puttftp") == 0) {
        // ------ Construct a Write Request (WRQ) and send to the server ------
        
        // Building the WRQ (Write Request)
        char *WRQ;
        int len = strlen(file) + strlen(mode) + 4;
        WRQ = malloc(len);
        WRQ[0] = 0;
        WRQ[1] = 2; // Opcode for WRQ
        strcpy(WRQ + 2, file);
        WRQ[2 + strlen(file)] = 0;
        strcpy(WRQ + 3 + strlen(file), mode);
        WRQ[3 + strlen(file) + strlen(mode)] = 0;

        // Send the WRQ to the server
        if ((messageLength = sendto(sockfd, WRQ, len, 0, (struct sockaddr *) cur->ai_addr, cur->ai_addrlen)) == -1) {
            perror("Client: 'sendto WRQ' error\n");
            exit(EXIT_FAILURE);
        }
        printf("CLIENT: Sent WRQ of %d bytes to the server\n", messageLength);

        // ------ Wait for acknowledgment from the server ------
        buf = malloc(BUFSIZE);
        if ((messageLength = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *) &addr, &slen)) == -1) {
            perror("Client: 'recvfrom' error\n");
            exit(EXIT_FAILURE);
        }
        if (*(buf + 1) == 5) {
            perror("Client: The server returned an error message\n");
            exit(EXIT_FAILURE);
        }
        printf("CLIENT: Acknowledgment for WRQ received from server: %d%d|%d%d\n", *(buf), *(buf + 1), *(buf + 2), *(buf + 3));

        // ------ Prepare the file for upload ------
        FILE *fileToSend = fopen(file, "rb");  // Open file for uploading
        if (fileToSend == NULL) {
            perror("CLIENT: Unable to open the file to send\n");
            exit(EXIT_FAILURE);
        }

        // ------ Send the file ------
        u_int16_t blockNumber = 1;
        char buffer[MAXDATA];
        size_t n = MAXDATA;
        do {
            // Read a chunk of the file
            n = fread(buffer, 1, MAXDATA, fileToSend);
            buffer[n] = '\0';
            char *packet = malloc(n + 4);
            packet[0] = 0;
            packet[1] = 3;  // Opcode for DATA packet
            packet[2] = 0;
            packet[3] = blockNumber;
            strcpy(packet + 4, buffer);

            // Send the data packet
            if ((messageLength = sendto(sockfd, packet, n + 4, 0, (struct sockaddr *) &addr, slen)) == -1) {
                perror("Client: 'sendto DATA' error\n");
                exit(EXIT_FAILURE);
            }
            printf("CLIENT: Sent packet number %d of %d bytes of data\n", blockNumber, messageLength - 4);

            // ------ Wait for acknowledgment from the server ------
            buf = malloc(BUFSIZE);
            if ((messageLength = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *) &addr, &slen)) == -1) {
                perror("Client: 'recvfrom' error\n");
                exit(EXIT_FAILURE);
            }
            printf("CLIENT: Acknowledgment for packet %d from server: %d%d|%d%d\n", *(buf + 3), *(buf), *(buf + 1), *(buf + 2), *(buf + 3));
            blockNumber += 1;
        } while (n == MAXDATA);  // Continue until file is fully sent

        fclose(fileToSend);
        printf("CLIENT: File transfer to the server completed\n");
    }

    return 0;
}

