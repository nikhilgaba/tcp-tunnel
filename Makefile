# compiler to use
CC=gcc

# enable debug symbols and warnings
CFLAGS=-c -Wall

all: server client tcptunnel

server: server.o
	$(CC) server.o -o server

client: client.o
	$(CC) client.o -o client

tcptunnel: tcptunnel.o
	$(CC) tcptunnel.o -o tcptunnel

server.o: server.c
	$(CC) $(CFLAGS) server.c

client.o: client.c
	$(CC) $(CFLAGS) client.c

tcptunnel.o: tcptunnel.c
	$(CC) $(CFLAGS) tcptunnel.c

clean:
	rm -rf *.o client server tcptunnel

