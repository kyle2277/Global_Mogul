#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "client_sockets.h"
#define BUFFER 1024
#define ERROR -1

/*
 * Initialize all socket descriptors and structures
 */
void init_sockets(char* argv[]) {
    if((sock_PI = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) {
        perror("client PI socket");
        exit(-1);
    }
    if((sock_DTP = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) {
        perror("client DTP socket");
        exit(-1);
    }

    //initialize remote server PI values
    remote_server_PI.sin_family = AF_INET;
    remote_server_PI.sin_port = htons(atoi(argv[2]));
    remote_server_PI.sin_addr.s_addr = inet_addr(argv[1]);
    bzero(&remote_server_PI.sin_zero, 8);
    //initialize remote server DTP values
    remote_server_DTP.sin_family = AF_INET;
    remote_server_DTP.sin_port = htons(atoi(argv[2])-1);
    remote_server_DTP.sin_addr.s_addr = inet_addr(argv[1]);
    bzero(&remote_server_DTP.sin_zero, 8);
}

/*
 * Connect to server PI port
 */
void connect_PI(int sockaddr_len) {
    if((connect(sock_PI, (struct sockaddr *)&remote_server_PI, sockaddr_len)) == ERROR) {
        perror("connect to server PI port");
        exit(-1);
    }
    printf("connected PI\n");
}

/*
 * Connect to server DTP port
 */
void connect_DTP(int sockaddr_len) {
    if((connect(sock_DTP, (struct sockaddr*)&remote_server_DTP, sockaddr_len)) == ERROR) {
        perror("connect to server DTP port");
        exit(-1);
    }
    printf("connected DTP\n");
}