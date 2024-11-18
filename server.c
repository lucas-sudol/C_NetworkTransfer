#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/stat.h>

#define DEFAULT_BUFSIZE 4096

char* strdup(char * old) {
    char * newString = malloc(sizeof(char) * (strlen(old) + 1));
    strcpy(newString, old);
    newString[strlen(old)] = '\0';
    return newString;
}

// Function to handle duplicate file names by prepending a unique prefix
char* get_unique_filename(char *filename, int* fileCtr) {
    struct stat buffer;
    char temp[256];

    if(stat(filename, &buffer) == 0) {
        snprintf(temp, sizeof(temp), "(%d)%s", ++*fileCtr, filename);
        return strdup(temp);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <port-number> [bufSize]\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    int bufSize = (argc == 3) ? atoi(argv[2]) : DEFAULT_BUFSIZE;
    
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[bufSize];
    int fileCtr = 0;
    char* newFilename;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        char fileName[256];
        int fileSize = 0;

        // Receive the file name
        int nameLen = recv(new_socket, fileName, sizeof(fileName), 0);
        if (nameLen <= 0) {
            perror("Failed to receive file name");
            close(new_socket);
            continue;
        }
        fileName[nameLen] = '\0';

        // Handle duplicate file names
        newFilename =  get_unique_filename(fileName, &fileCtr);

        if(newFilename == NULL)
            newFilename = fileName;
        
        FILE *file = fopen(newFilename, "wb");
        if (!file) {
            perror("File creation failed");
            close(new_socket);
            continue;
        }

        int bytesReceived;
        while ((bytesReceived = recv(new_socket, buffer, bufSize, 0)) > 0) {
            fwrite(buffer, 1, bytesReceived, file);
            fileSize += bytesReceived;
        }

        fclose(file);
        printf("Received file: %s (%d bytes) from %s with buffer size: %d bytes\n",
               fileName, fileSize, inet_ntoa(address.sin_addr), bufSize);

        close(new_socket);
        free(newFilename);
    }

    close(server_fd);
    return EXIT_SUCCESS;
}
