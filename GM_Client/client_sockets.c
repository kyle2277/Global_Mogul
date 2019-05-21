#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "client_sockets.h"
#define BUFFER 1024
#define ERROR -1
#define DEFAULT_PORT "60000"

//TODO switch strncpy for snprintf where applicable
//TODO consider changing socket commands to read/write

/*
 * Initialize all socket descriptors and structures
 */
void init_PI_socket() {
    // protocol set to 0 uses TCP
    if((sock_PI = socket(PF_INET, SOCK_STREAM, 0)) == ERROR) {
        perror("client PI socket");
        exit(-1);
    }

    //initialize remote server PI values
    remote_server_PI.sin_family = AF_INET;
    remote_server_PI.sin_addr.s_addr = inet_addr(server_addr);
    remote_server_PI.sin_port = htons(atoi(DEFAULT_PORT));
    bzero(&remote_server_PI.sin_zero, 8);
}

void init_DTP_socket(int port) {
    if((sock_DTP = socket(PF_INET, SOCK_STREAM, 0)) == ERROR) {
        perror("client DTP socket");
        exit(-1);
    }

    //initialize remote server DTP values
    remote_server_DTP.sin_family = AF_INET;
    remote_server_DTP.sin_addr.s_addr = inet_addr(server_addr);
    remote_server_DTP.sin_port = htons(port);
    bzero(&remote_server_DTP.sin_zero, 8);
}

/*
 * Connect to server PI port
 */
void connect_PI() {
    if((connect(sock_PI, (struct sockaddr *)&remote_server_PI, sockaddr_len)) == ERROR) {
        perror("connect to server PI port");
        exit(-1);
    }
    printf("connected PI\n");
}

/*
 * Connect to server DTP port
 */
void connect_DTP() {
    if((connect(sock_DTP, (struct sockaddr *)&remote_server_DTP, sockaddr_len)) == ERROR) {
        perror("connect to server DTP port");
        exit(-1);
    }
    printf("connected DTP\n");
}

void set_server_addr(char *address) {
    strncpy(server_addr, address, strlen(address));
}

void DTP_port(int port_num) {
    shutdown(sock_DTP, SHUT_RDWR);
    bzero(&remote_server_DTP, sizeof(remote_server_DTP));
    init_DTP_socket(port_num);
    connect_DTP();
}
