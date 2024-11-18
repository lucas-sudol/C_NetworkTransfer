#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define DEFAULT_BUFSIZE 4096

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage: %s <fileName> <IP-address/hostname>:<port-number> [bufSize]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *fileName = argv[1];
    char *addressPort = argv[2];
    int bufSize = (argc == 4) ? atoi(argv[3]) : DEFAULT_BUFSIZE;

    // Parse IP-address and port-number
    char *colon = strchr(addressPort, ':');
    if (colon == NULL) {
        fprintf(stderr, "Invalid address format. Use <IP-address/hostname>:<port-number>\n");
        exit(EXIT_FAILURE);
    }
    
    *colon = '\0'; // Split the string at the colon
    char *address = addressPort;
    int port = atoi(colon + 1);

    // Set up the file for reading
    FILE *file = fopen(fileName, "rb");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    int sockfd;
    struct addrinfo hints, *res;

    // Resolve the IP address (or hostname) using getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket

    if (getaddrinfo(address, NULL, &hints, &res) != 0) {
        perror("Error resolving hostname/IP address");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Set the port in the resolved address
    struct sockaddr_in *addr_in = (struct sockaddr_in *)res->ai_addr;
    addr_in->sin_port = htons(port);

    // Create socket and connect
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation error");
        fclose(file);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)addr_in, sizeof(struct sockaddr_in)) < 0) {
        perror("Connection failed");
        close(sockfd);
        fclose(file);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    // Send the file name first
    send(sockfd, fileName, strlen(fileName) + 1, 0); // +1 for null-terminator

    // Send the file content in chunks
    char *buffer = (char *)malloc(bufSize);
    int bytesRead;
    while ((bytesRead = fread(buffer, 1, bufSize, file)) > 0) {
        if (send(sockfd, buffer, bytesRead, 0) < 0) {
            perror("Error sending file data");
            break;
        }
    }

    free(buffer);
    fclose(file);
    close(sockfd);
    printf("File transfer completed.\n");
    return EXIT_SUCCESS;
}
