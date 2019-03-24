#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <dirent.h>
#include "authorize.h"

// pre-processor definitions
#define ERROR -1
#define MAX_CLIENTS 1
#define MAX_DATA 1024

/*
 * Initialization of all socket descriptors and structures
 */
void init_sockets(char *argv[], int sockaddr_len, struct sockaddr_in server_PI, struct sockaddr_in server_DTP) {
    // create PI and DTP sockets for server
    if((sock_PI = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) {
        perror("server PI socket");
        exit(-1);
    }
    if((sock_DTP = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) {
        perror("server DTP socket");
        exit(-1);
    }

    // initialize values in server_pi socket
    server_PI.sin_family = AF_INET;
    // htons = host byte order to network byte order
    server_PI.sin_port = htons(atoi(argv[1])); // atoi = ascii to integer
    server_PI.sin_addr.s_addr = INADDR_ANY; // listen on all interfaces on host at specified port sin_port
    bzero(&server_PI.sin_zero, 8);
    //initialize values in server_DTP socket
    server_DTP.sin_family = AF_INET;
    server_DTP.sin_port = htons(atoi(argv[1])-1);
    server_DTP.sin_addr.s_addr = INADDR_ANY;
    bzero(&server_DTP.sin_zero, 8);

    // bind the server PI socket to the PI port
    if((bind(sock_PI, (struct sockaddr *)&server_PI, sockaddr_len)) == ERROR) {
        perror("binding PI socket to port");
        exit(-1);
    }

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
void connect_PI(int sockaddr_len, struct sockaddr_in client_PI) {
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
void connect_DTP(int sockaddr_len, struct sockaddr_in client_DTP) {
    if((client_sock_DTP = accept(sock_DTP, (struct sockaddr *)&client_DTP, &sockaddr_len)) == ERROR) {
        perror("accept client DTP");
        exit(-1);
    }
    printf("client DTP accepted\n");
}

/*
 * receives data from client on PI channel and echoes data back on DTP channel.
 * DEPRECATED
 */
void echo_loop() {
    char data[MAX_DATA];
    int data_len = 1;
    // loop while client is connected to the server port and authorized
    while(data_len) {
        // wait for data from the client
        data_len = recv(client_sock_PI, data, MAX_DATA, 0);
        if(data_len) {
            data[data_len] = '\0';
            char* quit = strstr(data, "QUIT");
            // check if the user has terminated the client-side of the program
            if(quit) { break; }
            send(client_sock_DTP, data, data_len, 0);
            printf("Sent mesg: %s", data);
        }
    }
}

//todo write cases for specific commands
void command_loop() {
    printf("Waiting for client command.\n");
    char data[MAX_DATA];
    bool run = true;
    int data_len;
    while(run) {
        data_len = recv(client_sock_PI, data, MAX_DATA, 0);
        data[data_len] = '\0';
        if(strstr(data, "QUIT")) {
            printf("Client disconnected.\n");
            clean("Username", access_path);
            clean("Password", pass);
            shutdown(client_sock_PI, SHUT_RDWR);
            shutdown(client_sock_DTP, SHUT_RDWR);
            run = false;
        } else if(strstr(data, "ECHO")) {
            char* response = "confirm ECHO";
            printf("%s", response);
            send(client_sock_PI, response, strlen(response) , 0);
            echo_loop();
        } else if(strstr(data, "NOOP")) {
            char* response = "[200] command OK";
            send(client_sock_PI, response, strlen(response), 0);
        } else {
            char* response = "[500] syntax error, command unrecognized";
            send(client_sock_PI, response, strlen(response), 0);
        }
    }
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_PI;
    struct sockaddr_in server_DTP;
    struct sockaddr_in client_PI;
    struct sockaddr_in client_DTP;
    int sockaddr_len = sizeof(struct sockaddr_in);

    init_sockets(argv, sockaddr_len, server_PI, server_DTP);
    listen_PI();
    listen_DTP();

    while(true) {
        connect_PI(sockaddr_len, client_PI);
        get_auth();
        connect_DTP(sockaddr_len, client_DTP);
//        printf("Listening.\n");
        command_loop();
    }
}

/* POINTERS
 * '&' gives reference/address of a variable
 * '*' dereferences a pointer, gives value at address
 * FTP notes
 * Open one port for communication and another for file transfer
 * Sender converts the data from an internal character representation to the standard 8-bit NVT-ASCII
 * Get host IP: ip addr show eth0 | grep 'inet ' | awk '{print $2}' | cut -f1 -d'/'
 */

