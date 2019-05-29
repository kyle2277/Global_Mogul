#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "server_sockets.h"
#include "core.h"

// pre-processor definitions
#define ERROR -1
#define MAX_CLIENTS 1
#define MAX_DATA 1024
#define DEFAULT_PORT "60000"

/*
 * Initialization of all socket descriptors and structures
 */
void init_PI_socket() {
    // create PI socket for server
    //protocol set to 0 uses TCP
    if((sock_PI = socket(PF_INET, SOCK_STREAM, 0)) == ERROR) {
        perror("server PI socket");
        exit(-1);
    }

    // initialize values in server_pi socket
    server_PI.sin_family = AF_INET;
    // htons = host byte order to network byte order
    server_PI.sin_port = htons(atoi(DEFAULT_PORT)); // atoi = ascii to integer
    server_PI.sin_addr.s_addr = htonl(INADDR_ANY); // listen on all interfaces on host at specified port sin_port
    memset(&server_PI.sin_zero, '\0', 8);

    // bind the server PI socket to the PI port
    if((bind(sock_PI, (struct sockaddr *)&server_PI, sockaddr_len)) == ERROR) {
        perror("binding PI socket to port");
        exit(-1);
    }

}

void init_DTP_socket(int port) {
    // create DTP socket for server
    if((sock_DTP = socket(PF_INET, SOCK_STREAM, 0)) == ERROR) {
        perror("server DTP socket");
        exit(-1);
    }

    //initialize values in server_DTP socket
    server_DTP.sin_family = AF_INET;
    server_DTP.sin_port = htons(port);
    server_DTP.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&server_DTP.sin_zero, '\0', 8);

    //bind the server DTP socket to the DTP port
    if((bind(sock_DTP, (struct sockaddr *)&server_DTP, sockaddr_len)) == ERROR) {
        perror("binding DTP socket to port");
        exit(-1);
    }
}


/*
 * Instructs kernel to listen on server PI socket
 */
void listen_PI() {
    if((listen(sock_PI, MAX_CLIENTS)) == ERROR) {
        perror("listen_PI");
        exit(-1);
    }
}

/*
 * Instructs kernel to listen on server DTP socket
 */
void listen_DTP() {
    if((listen(sock_DTP, MAX_CLIENTS)) == ERROR) {
        perror("listen DTP");
        exit(-1);
    }
}

/*
 * Waiting for PI connection from client. Takes empty client structure, fills it with client info once connected
 * accept() returns new socket descriptor, used to send/receive data from this client
 */
void connect_PI() {
    if((client_sock_PI = accept(sock_PI, (struct sockaddr *)&client_PI, &sockaddr_len)) == ERROR) {
        perror("accept client PI");
        exit(-1);
    }
    printf("client PI accepted\n");
    // ntohs = network byte order to host byte order
    // inet_ntoa = network to ascii representation of IP address
    printf("New client connected from port no. %d and IP %s\n", ntohs(client_PI.sin_port), inet_ntoa(client_PI.sin_addr));
}

/*
 * Waiting for DTP connection from client
 */
void connect_DTP() {
    if((client_sock_DTP = accept(sock_DTP, (struct sockaddr *)&client_DTP, &sockaddr_len)) == ERROR) {
        perror("accept client DTP");
        exit(-1);
    }
    printf("client DTP accepted\n");
}

void DTP_port(int port_num) {
    shutdown(sock_DTP, SHUT_RDWR);
    memset(&server_DTP, '\0', sizeof(server_DTP));
    memset(&client_DTP, '\0', sizeof(client_DTP));
    init_DTP_socket(port_num);
    listen_DTP();
    connect_DTP();
}