#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

// pre-processor definitions
#define ERROR -1
#define MAX_CLIENTS 1
#define MAX_DATA 1024

typedef enum { false, true } bool;

int main(int argc, char *argv[]) {
    struct sockaddr_in server_PI;
    struct sockaddr_in server_DTP;
    struct sockaddr_in client_PI;
    struct sockaddr_in client_DTP;
    int sock_PI; // reference to the server's Protocol Interpreter (PI) socket which talks to the client
    int sock_DTP; // reference to the server's Data Transfer Process (DTP) socket which exchanges data with the client
    int client_sock_PI; // reference to connected client PI socket
    int client_sock_DTP; // reference to connected client DTP socket
    int sockaddr_len = sizeof(struct sockaddr_in);
    int data_len;
    char data[MAX_DATA];

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
    server_DTP.sin_port = htons(atoi(argv[1]-1));
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

    // instruct kernel to listen on this socket
    if((listen(sock_PI, MAX_CLIENTS)) == ERROR) {
        perror("listen");
        exit(-1);
    }

    while(true) {
        char input[MAX_DATA];
        fgets(input, MAX_DATA, stdin);
        if(strstr(input, "quit")) {
            break;
        }
        // waiting for PI connection from client. accept takes an empty client structure and when new client connects, this
        // is filled with the information from that client
        // returns new socket descriptor, used to send/receive data from this client
        if((client_sock_PI = accept(sock_PI, (struct sockaddr *)&client_PI, &sockaddr_len)) == ERROR) {
            perror("accept client PI");
            exit(-1);
        }
        //waiting for DTP connection from client
        if((client_sock_DTP = accept(sock_DTP, (struct sockaddr *)&client_DTP, &sockaddr_len)) == ERROR) {
            perror("accept client DTP");
            exit(-1);
        }

        // ntohs = network byte order to host byte order
        // inet_ntoa = network to ascii representation of IP address
        printf("New client connected from port no. %d and IP %s\n", ntohs(client_PI.sin_port), inet_ntoa(client_PI.sin_addr));

        data_len = 1;
         // loop for while client is connected to the server port
        while(data_len) {
            // wait for data from the client
            data_len = recv(client_sock_PI, data, MAX_DATA, 0);
            if(data_len) {
                char* quit = strstr(data, "quit");
                // check if the user has terminated the client-side of the program
                if(quit) {
                    break;
                }
                send(client_sock_DTP, data, data_len, 0);
                data[data_len] = '\0';
                printf("Sent mesg: %s", data);
            }
        }
        printf("Client disconnected.\n");
        shutdown(client_sock_PI, SHUT_RDWR);
        shutdown(client_sock_DTP, SHUT_RDWR);
    }

    printf("Program termination.\n");
    shutdown(sock_PI, SHUT_RDWR);
    shutdown(sock_DTP, SHUT_RDWR);

}


/* FTP notes
 * Open one port for communication and another for file transfer
 * Sender converts the data from an internal character representation to the standard 8-bit NVT-ASCII
 * Get host IP: ip addr show eth0 | grep 'inet ' | awk '{print $2}' | cut -f1 -d'/'
 */
