#include <netinet/in.h>
#include <time.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAXLINE     4096    /* max text line length */
#define LISTENQ     1024    /* 2nd argument to listen() */
//#define DAYTIME_PORT 3333
int DAYTIME_PORT;

int checkNumberOfArguments(int argc) {
    if (argc != 2) {
        printf("usage: server <portNumber>\n");
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

int getClientIpAddress(int connfd, struct sockaddr_in *client) {
    //struct sockaddr_in result;
    int returnStatus;
    socklen_t length;
    length = sizeof(*client);
    char clientIp[INET_ADDRSTRLEN];
    returnStatus = getpeername(connfd,(struct sockaddr*)client,&length);
    if (returnStatus != 0) {
        printf("getpeername: %s\n", gai_strerror(returnStatus));
        return -1;
    }
    else {
        inet_ntop(AF_INET,&client->sin_addr,clientIp,INET_ADDRSTRLEN);
        printf("IP Address of Client: %s\n", clientIp);
        return 0;
    }

}

int getClientHostName(struct sockaddr_in *client) {
    int returnStatus;
    char clientName[1024];
    returnStatus = getnameinfo((struct sockaddr *)client,sizeof(*client),clientName,sizeof(clientName),NULL,0,0);
    if (returnStatus != 0) {
        printf("getnameinfo: %s\n", gai_strerror(returnStatus));
        return -1;
    }
    else {
        printf("Server Name of Client: %s\n", clientName);
        return 0;
    }
}

int main(int argc, char **argv)
{
    int     listenfd, connfd;
    struct sockaddr_in servaddr;
    char    buff[MAXLINE];
    time_t ticks;

    int correctNumOfArguments = checkNumberOfArguments(argc);
    if (correctNumOfArguments == -1) {
        exit(1);
    }

    int correctPortNumber = checkPortNumber(argv[1]);
    if (correctPortNumber == -1) {
        exit(1);
    }

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(DAYTIME_PORT); /* daytime server */

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

    for ( ; ; ) {
        struct sockaddr_in client;
        bzero(&client,sizeof(client));
        connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);
        int clientIpAddressReturn = getClientIpAddress(connfd,&client);
        if (clientIpAddressReturn == -1) {
            printf("Could not get client IP address");
        }
        int clientHostNameReturn = getClientHostName(&client);
        if (clientHostNameReturn == -1) {
            printf("Could not get client name");
        }
        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        write(connfd, buff, strlen(buff));
        printf("Sending response: %s", buff);

        close(connfd);
    }
}

