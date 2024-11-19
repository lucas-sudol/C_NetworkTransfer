#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define DEFAULT_BUFSIZE 4096

char* strdup(char * old) {
    char * newString = malloc(sizeof(char) * (strlen(old) + 1));
    strcpy(newString, old);
    newString[strlen(old)] = '\0';
    return newString;
}

// Adds unique prefix
char* get_unique_filename(char *filename, int* fileCtr) {
    struct stat buffer;
    char temp[256];

    if(stat(filename, &buffer) == 0) {
        snprintf(temp, sizeof(temp), "%d-%s", ++*fileCtr, filename);
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
    int bufSize = DEFAULT_BUFSIZE;
    
    if (argc == 3) {
        bufSize = atoi(argv[2]);
    }
    
    int mysocket, consocket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[bufSize];
    int fileCtr = 0;
    char* newFilename;

    if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        fprintf(stderr, "Socket failed\n");
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(mysocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        fprintf(stderr, "Bind failed\n");
        close(mysocket);

        return 1;
    }

    if (listen(mysocket, 3) < 0) {
        fprintf(stderr, "Listen failed\n");
        close(mysocket);

        return 1;
    }

    while(1) {
        if ((consocket = accept(mysocket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            fprintf(stderr, "Accept failed\n");
            continue;
        }

        char fileName[256];
        int fileSize = 0;

        // Receive the file name
        int nameLen = recv(consocket, fileName, sizeof(fileName), 0);
        if (nameLen <= 0) {
            fprintf(stderr, "Failed to receive file name\n");
            close(consocket);
            continue;
        }
        fileName[nameLen] = '\0';

        // Handle duplicate file names
        newFilename =  get_unique_filename(fileName, &fileCtr);

        if(newFilename == NULL) {
            newFilename = fileName;
        }
        
        FILE *file = fopen(newFilename, "wb");
        if (!file) {
            fprintf(stderr, "File creation failed\n");
            close(consocket);
            continue;
        }

        int bytesReceived;
        while ((bytesReceived = recv(consocket, buffer, bufSize, 0)) > 0) {
            fwrite(buffer, 1, bytesReceived, file);
            fileSize += bytesReceived;
        }

        printf("Received file: %s (%d bytes) from %s with buffer size: %d bytes\n", fileName, fileSize, inet_ntoa(address.sin_addr), bufSize);

        fclose(file);
        close(consocket);
        free(newFilename);
    }

    close(mysocket);
    return 0;
}
