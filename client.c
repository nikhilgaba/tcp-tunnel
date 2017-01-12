#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE     4096    /* max text line length */
//#define DAYTIME_PORT 3333
int DAYTIME_PORT;

int checkNumberOfArguments(int argc) {
    if (argc != 3) {
        printf("usage: client <Ipaddress/HostName> <portNumber>\n");
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
        DAYTIME_PORT = port_num;
        return 0;
    }
}

int
main(int argc, char **argv)
{
    int     sockfd, n;
    char    recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;

    int correctNumOfArguments = checkNumberOfArguments(argc);
    if (correctNumOfArguments == -1) {
        exit(1);
    }

    int correctPortNumber = checkPortNumber(argv[2]);
    if (correctPortNumber == -1) {
        exit(1);
    }

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket error\n");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(DAYTIME_PORT);  /* daytime server */
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        printf("inet_pton error for %s\n", argv[1]);
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        printf("connect error\n");
        exit(1);
    }

    while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;        /* null terminate */
        if (fputs(recvline, stdout) == EOF) {
            printf("fputs error\n");
            exit(1);
        }
    }
    if (n < 0) {
        printf("read error\n");
        exit(1);
    }

    exit(0);
}

