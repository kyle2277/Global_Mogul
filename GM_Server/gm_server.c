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
#define MAX_STR_LEN 80

typedef enum { false, true } bool;
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
char access_path[MAX_STR_LEN];
char pass[MAX_STR_LEN];

void init_sockets(char *argv[]) {
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

void listen_PI() {
    // instruct kernel to listen on PI socket
    if((listen(sock_PI, MAX_CLIENTS)) == ERROR) {
        perror("listen_PI");
        exit(-1);
    }
}

void listen_DTP() {
    //instructs kernel to listen on DTP socket
    if((listen(sock_DTP, MAX_CLIENTS)) == ERROR) {
        perror("listen DTP");
        exit(-1);
    }
}

void connect_PI() {
    // waiting for PI connection from client. accept takes an empty client structure and when new client connects, this
    // is filled with the information from that client
    // returns new socket descriptor, used to send/receive data from this client
    if((client_sock_PI = accept(sock_PI, (struct sockaddr *)&client_PI, &sockaddr_len)) == ERROR) {
        perror("accept client PI");
        exit(-1);
    }
    printf("client PI accepted\n");
    // ntohs = network byte order to host byte order
    // inet_ntoa = network to ascii representation of IP address
    printf("New client connected from port no. %d and IP %s\n", ntohs(client_PI.sin_port), inet_ntoa(client_PI.sin_addr));
}

void connect_DTP() {
    //waiting for DTP connection from client
    if((client_sock_DTP = accept(sock_DTP, (struct sockaddr *)&client_DTP, &sockaddr_len)) == ERROR) {
        perror("accept client DTP");
        exit(-1);
    }
    printf("client DTP accepted\n");
}

void get_auth() {
    while (!*access_path | !*pass) {
        char usr_input[MAX_DATA];
        int auth_len;
        auth_len = recv(client_sock_PI, usr_input, MAX_DATA, 0);
        // if anything is received form the client, proceed with authorization
        if (auth_len) {
            usr_input[auth_len] = '\0';
            bool skip = false;
            int max_args = 2;
            char *delim = " ";
            char *token = strtok(usr_input, delim);
            char *args[max_args];
            int token_count = 0;
            args[token_count] = token;
            while (token != NULL) {
                token_count++;
                token = strtok(NULL, delim);
                if (token_count > max_args) {
                    send(client_sock_PI, "Too many args.", MAX_DATA, 0);
                    skip = true;
                    break;
                }
                if (token != NULL) {
                    args[token_count] = token;
                }
            }
            if(!skip) {
                if (token_count < max_args) {
                    send(client_sock_PI, "Too few args.", MAX_DATA, 0);
                    get_auth();
                }
                strncpy(access_path, args[0], strlen(args[0]));
                strncpy(pass, args[1], strlen(args[1]));
                printf("%s, ", access_path);
                printf("%s", pass);
            }

        }
    }
    // 230: user logged in, proceed
    send(client_sock_PI, "230", MAX_DATA, 0);
}

void clear_auth() {
    // iterate through credential strings in memory and set all to null
    NULL;
}

void echo_loop() {
    data_len = 1;
    // loop while client is connected to the server port and authorized
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
}

int main(int argc, char *argv[]) {
    init_sockets(argv);
    listen_PI();
    listen_DTP();

    while(true) {
//        char input[MAX_DATA];
//        fgets(input, MAX_DATA, stdin);
//        if(strstr(input, "quit")) {
//            break;
//        }
        connect_PI();
        connect_DTP();
        get_auth();
        printf("Listening.\n");
        echo_loop();
        printf("Client disconnected.\n");
        //clear_auth();
        shutdown(client_sock_PI, SHUT_RDWR);
        shutdown(client_sock_DTP, SHUT_RDWR);

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

