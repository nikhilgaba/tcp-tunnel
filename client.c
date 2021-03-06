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

int checkNumberOfArguments(int argc) {
    if (argc != 3 && argc != 5) {
        printf("usage: client [<Tunnel Ipaddress/HostName>] [<Tunnel portNumber>] <Server Ipaddress/HostName> <Server portNumber>\n");
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
        printf("incorrect port numbers\n");
        printf("select ports between 1024 and 65535\n");
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
        //printf("resolved hostname\n");
        freeaddrinfo(res);
        return 0;
    }

}

void printIpAddress(struct sockaddr_in *servaddr) {
    char result[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,&servaddr->sin_addr,result,INET_ADDRSTRLEN);
    printf("IP Address: %s\n", result);
}

int printServerName(struct sockaddr_in *servaddr) {
    int returnStatus;
    char serverName[1024];
    returnStatus = getnameinfo((struct sockaddr *)servaddr,sizeof(*servaddr),serverName,sizeof(serverName),NULL,0,0);
    if (returnStatus != 0) {
        printf("getnameinfo: %s\n", gai_strerror(returnStatus));
        return -1;
    }
    else {
        printf("Server Name: %s\n", serverName);
        return 0;
    }
}

int getServerName(struct sockaddr_in *servaddr, char *name) {
    int returnStatus;
    char serverName[1024];
    returnStatus = getnameinfo((struct sockaddr *)servaddr,sizeof(*servaddr),serverName,sizeof(serverName),NULL,0,0);
    if (returnStatus != 0) {
        printf("getnameinfo: %s\n", gai_strerror(returnStatus));
        return -1;
    }
    else {
        strcpy(name,serverName);
        return 0;
    }
}

void getIPAddress(struct sockaddr_in *servaddr, char *ip) {
    char result[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,&servaddr->sin_addr,result,INET_ADDRSTRLEN);
    strcpy(ip, result);
}

void getServerInfo(char *server, char *serverPort, char *serverName, char *serverIP) {
    int     serverServerPort;
    struct sockaddr_in servaddr;

    serverServerPort = atoi(serverPort);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(serverServerPort);

    if (inet_pton(AF_INET, server, &servaddr.sin_addr) <= 0) {

        convertHostNameToIp(server, &servaddr);
    }

    getServerName(&servaddr, serverName);
    getIPAddress(&servaddr, serverIP);
}

int setupConnectionWithTunnel(char *tunnel, char *tunnelPort, char *tunnelName, char *tunnelIP) {
    int     sockfd, tunnelServerPort;
    struct sockaddr_in tunneladdr;

    tunnelServerPort = atoi(tunnelPort);

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket error\n");
        return -1;
    }

    bzero(&tunneladdr, sizeof(tunneladdr));
    tunneladdr.sin_family = AF_INET;
    tunneladdr.sin_port = htons(tunnelServerPort); 
    if (inet_pton(AF_INET, tunnel, &tunneladdr.sin_addr) <= 0) {
        //printf("inet_pton error for %s\n", tunnel);
        //printf("trying to resolve hostname for tunnel %s\n", tunnel);

        int isValidHostName = convertHostNameToIp(tunnel, &tunneladdr);
        if (isValidHostName == -1) {
            return -1;
        }
    }

    if (connect(sockfd, (struct sockaddr *) &tunneladdr, sizeof(tunneladdr)) < 0) {
        printf("connect error\n");
        return -1;
    }

    getServerName(&tunneladdr, tunnelName);
    getIPAddress(&tunneladdr, tunnelIP);

    return sockfd;
}

int writeToTunnel(int tunnelfd, char *serverName, char *serverPort) {
    char sendString[MAXLINE];
    strcpy(sendString,"\0");
    strcat(sendString,serverName);
    strcat(sendString,"|");
    strcat(sendString,serverPort);
    int writeReturnCode = write(tunnelfd, sendString, strlen(sendString));
    if (writeReturnCode == -1) {
        printf("could not send to tunnel\n");
        return -1;
    }
    else {
        return 0;
    }
}

int readFromTunnel(int tunnelfd, char *timeFromServer) {
    int n;
    n = read(tunnelfd, timeFromServer, MAXLINE);
    if (n < 0) {
        printf("read error\n");
        return -1;
    }
    else {
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

int runDirectConnection (char **argv) {
    int     sockfd, n, serverPort;
    char    recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;

    serverPort = atoi(argv[2]);

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket error\n");
        return -1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(serverPort);  /* daytime server */
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        //printf("inet_pton error for %s\n", argv[1]);
        //printf("trying to resolve hostname %s\n", argv[1]);

        int isValidHostName = convertHostNameToIp(argv[1], &servaddr);
        if (isValidHostName == -1) {
            return -1;
        }
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        printf("connect error\n");
        return -1;
    }

    int serverNameResultCode = printServerName(&servaddr);
    if (serverNameResultCode == -1) {
        return -1;
    }
    printIpAddress(&servaddr);
    printf("Time: ");
    while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;        /* null terminate */
        if (fputs(recvline, stdout) == EOF) {
            printf("fputs error\n");
            return -1;
        }
    }
    if (n < 0) {
        printf("read error\n");
        return -1;
    }

    return 0;
}

int runConnectionViaTunnel(char **argv) {
    char timeFromServer[MAXLINE+1];
    char tunnelName[MAXLINE];
    char tunnelIP[MAXLINE];

    char serverName[MAXLINE];
    char serverIP[MAXLINE];

    getServerInfo(argv[3], argv[4], serverName, serverIP);


    int tunnelConnection = setupConnectionWithTunnel(argv[1], argv[2], tunnelName, tunnelIP);
    if (tunnelConnection == -1) {
        closeConnection(tunnelConnection);
        return -1;
    }

    int writeToTunnelReturnStatus = writeToTunnel(tunnelConnection,argv[3],argv[4]);
    if (writeToTunnelReturnStatus == -1) {
        closeConnection(tunnelConnection);
        return -1;
    }

    int readFromTunnelReturnStatus = readFromTunnel(tunnelConnection, timeFromServer);
    if  (readFromTunnelReturnStatus == -1) {
        closeConnection(tunnelConnection);
        return -1;
    }

    closeConnection(tunnelConnection);
    printf("Server Name:  %s\n", serverName);
    printf("IP Address:  %s\n", serverIP);
    printf("Time: %s\n", timeFromServer);
    printf("Via Tunnel: %s\n", tunnelName);
    printf("IP Address: %s\n", tunnelIP);
    printf("Port Number: %s\n", argv[2]);
    return 0;
}

int main(int argc, char **argv)
{
    int correctNumOfArguments = checkNumberOfArguments(argc);
    if (correctNumOfArguments == -1) {
        exit(1);
    }

    if (argc == 3) {
        int correctPortNumber = checkPortNumber(argv[2]);
        if (correctPortNumber == -1) {
            exit(1);
        }
        else {
            int directConnectionReturnCode = runDirectConnection(argv);
            if (directConnectionReturnCode == -1) {
                exit(1);
            }
        }

    }
    else if (argc == 5) {
        int correctPortNumberTunnel = checkPortNumber(argv[2]);
        int correctPortNumberServer = checkPortNumber(argv[4]);
        if (correctPortNumberTunnel == -1 || correctPortNumberServer == -1) {
            exit(1);
        }
        else {
            int connectionViaTunnelReturnCode = runConnectionViaTunnel(argv);
            if (connectionViaTunnelReturnCode == -1) {
                exit(1);
            }
        }
    }

    exit(0);
}

