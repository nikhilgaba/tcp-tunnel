#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define MAXLINE     4096    /* max text line length */
#define LISTENQ     1024    /* 2nd argument to listen() */

int checkNumberOfArguments(int argc) {
    if (argc != 2) {
        printf("usage: tcptunnel <listen portNumber>\n");
        return -1;
    }
    else {
        return 0;
    }
}

int checkPortNumber(char *arg) {
    int port_num;
    port_num = atoi(arg);
    if (port_num < 1024 || port_num > 65535) {
        printf("incorrect port number\n");
        printf("select port between 1024 and 65535\n");
        return -1;
    }
    else {

        return 0;
    }
}

int listenForClient(char *arg) {
	int     listenfd, connfd, serverPort;
    struct sockaddr_in servaddr;

    serverPort = atoi(arg);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(serverPort);

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

        struct sockaddr_in client;
        bzero(&client,sizeof(client));
        connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);

        return connfd;
}

int main(int argc, char **argv)
{
	//char recvline[MAXLINE + 1];
	//int n;

	int correctNumOfArguments = checkNumberOfArguments(argc);
    if (correctNumOfArguments == -1) {
        exit(1);
    }

    int correctPortNumber = checkPortNumber(argv[1]);
    if (correctPortNumber == -1) {
        exit(1);
    }

    int clientConnection = listenForClient(argv[1]);

    /*while ( (n = read(clientConnection, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            printf("fputs error\n");
            return -1;
        }
    }
    if (n < 0) {
        printf("read error\n");
        return -1;
    }*/

	exit(0);
} 