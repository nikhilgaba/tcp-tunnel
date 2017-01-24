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

int convertHostNameToIp(char *hostName, struct sockaddr_in *servaddr ) {
    int result;
    struct addrinfo hints, *res;
    bzero(&hints,sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    result = getaddrinfo(hostName,NULL,&hints,&res);
    if (result != 0) {
        printf("getaddrinfo: %s\n", gai_strerror(result));
        freeaddrinfo(res);
        return -1;
    }
    else {
        servaddr->sin_addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
        //printf("resolved hostname to %s\n", inet_ntoa(servaddr->sin_addr));
        //printf("resolved hostname\n");
        freeaddrinfo(res);
        return 0;
    }

}

int listenForClient(char *arg) {
	int     listenfd, serverPort;
    struct sockaddr_in servaddr;

    serverPort = atoi(arg);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(serverPort);

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

    return listenfd;
}

int waitForClientToConnect(int listenfd) {
    struct sockaddr_in client;
    int connfd;
    bzero(&client,sizeof(client));
    connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);

    return connfd;
}

int connectToServer(char *serverName, char *serverPort) {
    int     sockfd, serverServerPort;
    struct sockaddr_in servaddr;

    serverServerPort = atoi(serverPort);

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket error\n");
        return -1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(serverServerPort); 
    if (inet_pton(AF_INET, serverName, &servaddr.sin_addr) <= 0) {
        //printf("inet_pton error for %s\n", serverName);
        //printf("trying to resolve hostname for server %s\n", serverName);

        int isValidHostName = convertHostNameToIp(serverName, &servaddr);
        if (isValidHostName == -1) {
            return -1;
        }
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        printf("connect error\n");
        return -1;
    }

    return sockfd;
}

int readFromServer(int serverfd, char *timeFromServer) {
    int n;
    n = read(serverfd, timeFromServer, MAXLINE);
    if (n < 0) {
        printf("read error\n");
        return -1;
    }
    else {
        return 0;
    }

}

int readFromClientAndParseString(int clientfd, char *serverName, char *serverPort) {
	int n;
	char buff[MAXLINE];
    n = read(clientfd, buff, MAXLINE);
    if (n < 0) {
        printf("read error\n");
        return -1;
    }
    else {
    	strcpy(serverName, strtok(buff, "|"));
    	strcpy(serverPort, strtok(NULL, "|"));
        return 0;
    }
}

int closeConnection(int fd) {
    int closeReturnStatus = close(fd);
    if (closeReturnStatus == -1) {
        printf("could not close connection\n");
        return -1;
    }
    else {
        return 0;
    }
}

int sendTimeToClient(int clientfd, char *timeFromServer) {
    int writeReturnCode = write(clientfd, timeFromServer, strlen(timeFromServer));
    if (writeReturnCode == -1) {
    	printf("could not send to client\n");
        return -1;
    }
    else {
        return 0;
    }
}

int main(int argc, char **argv)
{
	char timeFromServer[MAXLINE + 1];
	char serverName[MAXLINE];
	char serverPort[MAXLINE];

	int correctNumOfArguments = checkNumberOfArguments(argc);
    if (correctNumOfArguments == -1) {
        exit(1);
    }

    int correctPortNumber = checkPortNumber(argv[1]);
    if (correctPortNumber == -1) {
        exit(1);
    }
    int listenfd = listenForClient(argv[1]);

    for ( ; ; ) {
        int clientConnection = waitForClientToConnect(listenfd);

        int readFromClientAndParseStringReturnCode = readFromClientAndParseString(clientConnection,
            serverName,serverPort);
        if (readFromClientAndParseStringReturnCode == -1) {
            closeConnection(clientConnection);
            continue;
            //exit(1);
        }

        int serverConnection = connectToServer(serverName, serverPort);
        if (serverConnection == -1) {
            closeConnection(clientConnection);
            closeConnection(serverConnection);
            continue;
            //exit(1);
        }

        int readFromServerReturnCode = readFromServer(serverConnection, timeFromServer);
        if (readFromServerReturnCode == -1) {
            closeConnection(clientConnection);
            closeConnection(serverConnection);
            continue;
            //exit(1);
        }

        closeConnection(serverConnection);

        int sendTimeReturnStatus = sendTimeToClient(clientConnection,timeFromServer);
        if (sendTimeReturnStatus == -1) {
            closeConnection(clientConnection);
            continue;
            //exit(1);
        }

        closeConnection(clientConnection);
    }
} 