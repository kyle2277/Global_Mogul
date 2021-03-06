#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client_sockets.h"
#define BUFFER 1024
#define DEFAULT_PORT "60000"

#ifdef _WIN32
//Windows systems
//#pragma comment(lib,"ws2_32.lib") //Winsock Library
#include <winsock2.h>
#define SOCK_ERROR SOCKET_ERROR
#define GET_ERROR WSAGetLastError()

void init_Winsock() {
    WSADATA wsaData;
    int iResult;
    //Initialize Winsock
    if((iResult = WSAStartup(MAKEWORD(2,2), &wsaData) != 0)) {
        printf("WSAStartup failed: %d\n", iResult);
    } else {
        printf("Winsock Initialized.\n");
    }
}

void resetErrno(){};

void DTP_port(int port_num) {
    closesocket(sock_DTP);
    WSACleanup();
    memset(&remote_server_DTP, '\0', sizeof(remote_server_DTP));
    init_DTP_socket(port_num);
    connect_DTP();
}

void shutdownAll() {
    closesocket(sock_PI);
    closesocket(sock_DTP);
    WSACleanup();
}
#else
//Linux and Mac OSX systems
#include <arpa/inet.h>
#include <errno.h>
#define GET_ERROR errno
#define SOCK_ERROR -1

void resetErrno() {
    errno = 0;
}

void DTP_port(int port_num) {
    shutdown(sock_DTP, SHUT_RDWR);
    memset(&remote_server_DTP, '\0', sizeof(remote_server_DTP));
    init_DTP_socket(port_num);
    connect_DTP();
}

void shutdownAll() {
    shutdown(sock_PI, SHUT_RDWR);
    shutdown(sock_DTP, SHUT_RDWR);
}
#endif

//TODO switch strncpy for snprintf where applicable
//TODO consider changing socket commands to read/write

/*
 * Initialize all socket descriptors and structures
 */
void init_PI_socket() {
    resetErrno();
    // protocol set to 0 uses TCP
    if((sock_PI = socket(PF_INET, SOCK_STREAM, 0)) == SOCK_ERROR) {
        printf("Failure to connect client PI socket: %d\n", GET_ERROR);
        exit(-1);
    }

    //initialize remote server PI values
    remote_server_PI.sin_family = AF_INET;
    remote_server_PI.sin_addr.s_addr = inet_addr(server_addr);
    remote_server_PI.sin_port = htons(atoi(DEFAULT_PORT));
    memset(&remote_server_PI.sin_zero, '\0', 8);
}

void init_DTP_socket(int port) {
    resetErrno();
    if((sock_DTP = socket(PF_INET, SOCK_STREAM, 0)) == SOCK_ERROR) {
        printf("Failure to connect client DTP socket: %d\n", GET_ERROR);
        exit(-1);
    }

    //initialize remote server DTP values
    remote_server_DTP.sin_family = AF_INET;
    remote_server_DTP.sin_addr.s_addr = inet_addr(server_addr);
    remote_server_DTP.sin_port = htons(port);
    memset(&remote_server_DTP.sin_zero, '\0', 8);
}

/*
 * Connect to server PI port
 */
void connect_PI() {
    if((connect(sock_PI, (struct sockaddr *)&remote_server_PI, sockaddr_len)) == SOCK_ERROR) {
        resetErrno();
        printf("Failure to connect to server PI port: %d\n", GET_ERROR);
        exit(-1);
    }
    printf("connected PI\n");
}

/*
 * Connect to server DTP port
 */
void connect_DTP() {
    resetErrno();
    if((connect(sock_DTP, (struct sockaddr *)&remote_server_DTP, sockaddr_len)) == SOCK_ERROR) {
        printf("Failure to connect to server DTP port: %d\n", GET_ERROR);
        exit(-1);
    }
    printf("connected DTP\n");
}

void set_server_addr(char *address) {
    strncpy(server_addr, address, strlen(address));
}


