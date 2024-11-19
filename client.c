#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define DEFAULT_BUFSIZE 4096

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage: %s <fileName> <IP-address/hostname>:<port-number> [bufSize]\n", argv[0]);
        return 1;
    }

    char *fileName = argv[1];
    char *addressPort = argv[2];
    int bufSize = DEFAULT_BUFSIZE;
    
    if (argc == 4) {
        bufSize = atoi(argv[3]);
    }

    // Check if input format is valid
    char *colon = strchr(addressPort, ':');
    if (colon == NULL) {
        fprintf(stderr, "Invalid address format. Use <IP-address/hostname>:<port-number>\n");
        return 1;
    }
    
    *colon = '\0'; // Split the string at the colon
    char *address = addressPort;
    int port = atoi(colon + 1);

    // Open file
    FILE *file = fopen(fileName, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error opening file\n");
        return 1;
    }

    int mysocket;
    struct addrinfo hints, *res;

    // Resolve host using getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;          // IPv4
    hints.ai_socktype = SOCK_STREAM;    // TCP

    if (getaddrinfo(address, NULL, &hints, &res) != 0) {
        fprintf(stderr, "Error resolving hostname/IP address\n");
        fclose(file);
        return 1;
    }

    struct sockaddr_in *dest = (struct sockaddr_in *)res->ai_addr;
    dest->sin_port = htons(port); // fix

    // Create socket and connect
    mysocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mysocket < 0) {
        fprintf(stderr, "Socket creation error\n");

        fclose(file);
        freeaddrinfo(res);
        return 1;
    }

    if (connect(mysocket, (struct sockaddr *)dest, sizeof(struct sockaddr_in)) < 0) {
        fprintf(stderr, "Connection failed\n");

        close(mysocket);
        fclose(file);
        freeaddrinfo(res);
        return 1;
    }

    freeaddrinfo(res);

    // Send the file name first
    send(mysocket, fileName, strlen(fileName) + 1, 0); // +1 for \0

    // Send the file content in chunks
    char * buffer = malloc(bufSize);
    int bytesRead;

    while ((bytesRead = fread(buffer, 1, bufSize, file)) > 0) {
        if (send(mysocket, buffer, bytesRead, 0) < 0) {
            fprintf(stderr, "Error sending file data\n");
            break;
        }
    }

    free(buffer);
    fclose(file);
    close(mysocket);
    printf("File transfer completed.\n");
    return 0;
}
