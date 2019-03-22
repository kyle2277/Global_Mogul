#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#define ERROR -1
#define BUFFER 1024

//todo clean up global variables

typedef enum { false, true } bool;
struct sockaddr_in remote_server_PI;
struct sockaddr_in remote_server_DTP;
int sock_PI; // socket descriptor for the client Process Interpreter (PI) socket
int sock_DTP; // socket descriptor for the client Data Transfer Process (STP) socket
char pass[BUFFER];

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

void clean_pass() {
    int len = strlen(pass);
    for(int i = 0; i < len; i++) { pass[i] = '\0'; }
    printf("Password cleared.\n");
}

/*
 * Sets password for decryption purposes
 */
void set_pass(char input[]) {
    char *delim = " ";
    input[strlen(input)] = '\0';
    strtok(input, delim);
    char *token = strtok(NULL, delim);
    token[(int)strlen(token)-1] = '\0';
    strncpy(pass, token, strlen(token));
}

/*
 * Accepts user credentials from console and sends to server via PI connection
 */
void send_auth() {
    char input[BUFFER]; // stores user input
    char auth_reply[BUFFER]; // stores server response
    printf("[332] Authorization required.\n");
    fgets(input, BUFFER, stdin);
    send(sock_PI, input, strlen(input), 0);
    int reply_len;
    reply_len = recv(sock_PI, auth_reply, BUFFER, 0);
    if (reply_len) {
        auth_reply[reply_len] = '\0';
        if(strstr(input, "PASS") && (strstr(auth_reply, "333") || strstr(auth_reply, "230"))) { set_pass(input); }
        printf("%s\n", auth_reply);
        if(!strstr(auth_reply, "230")) { send_auth(); }
    } else {
        send_auth();
    }
}

/* DEPRECATED */
void echo_loop() {
    int reply_len;
    char input[BUFFER]; // stores user input
    char output[BUFFER]; // stores remote_server response
    bool run = true;
    while(run) {
        // fgets() reads input (containing spaces) from user, stores in provided string (input)
        fgets(input, BUFFER, stdin);
        char* quit = strstr(input, "QUIT");
        send(sock_PI, input, strlen(input), 0);
        // check if the user wants to terminate the program
        if(quit) {
            printf("Connection terminated.\n");
            run = false;
        }
        reply_len = recv(sock_DTP, output, BUFFER, 0);
        output[reply_len] = '\0';
        printf("%s", output);
    }
}

bool dispatch(char input[]) {
    if(strstr(input, "ECHO")) {
        echo_loop();
        return true;
    } else if(strstr(input, "QUIT")) {
        return false;
    }
}

void command_loop() {
    printf("Ready.\n");
    char input[BUFFER];
    char response[BUFFER];
    int response_len;
    bool run = true;
    while(run) {
        // fgets() reads input (containing spaces) from user, stores in provided string (input)
        fgets(input, BUFFER, stdin);
        char* quit = strstr(input, "QUIT");
        send(sock_PI, input, strlen(input), 0);
        // check if the user wants to terminate the program
        if(quit) {
            printf("Connection terminated.\n");
            run = false;
        }
        response_len = recv(sock_DTP, response, BUFFER, 0);
        response[response_len] = '\0';
        printf("%s", response);
        run = dispatch(input);
    }
    clean_pass();
    shutdown(sock_PI, SHUT_RDWR);
    shutdown(sock_DTP, SHUT_RDWR);
}

int main(int argc, char *argv[]) {
    int sockaddr_len = sizeof(struct sockaddr_in);
    init_sockets(argv);
    connect_PI(sockaddr_len);
    send_auth();
    connect_DTP(sockaddr_len);
    command_loop();
    return 0;
}

/* FTP notes
 * Open one port for communication with server and another for file transfer
 * Receiver will convert the data from the standard form to his own internal form.
 *
 */