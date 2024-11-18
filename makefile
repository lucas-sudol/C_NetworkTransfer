CC = gcc
CFLAGS = -Wall -std=c99
TARGETS = server sendFile

all: $(TARGETS)

server: server.c
	$(CC) $(CFLAGS) -o server server.c

sendFile: client.c
	$(CC) $(CFLAGS) -o sendFile client.c

clean:
	rm -f $(TARGETS)
