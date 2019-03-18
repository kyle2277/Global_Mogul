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

typedef enum { false, true } bool;
struct sockaddr_in remote_server_PI;
struct sockaddr_in remote_server_DTP;
int sock_PI; // socket descriptor for the client Process Interpreter (PI) socket
int sock_DTP; // socket descriptor for the client Data Transfer Process (STP) socket
char input[BUFFER]; // stores user input
char output[BUFFER]; // stores remote_server response
int sockaddr_len = sizeof(struct sockaddr_in);
int len;

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

void connect_PI() {
    //connect to server PI port
    if((connect(sock_PI, (struct sockaddr *)&remote_server_PI, sockaddr_len)) == ERROR) {
        perror("connect to server PI port");
        exit(-1);
    }
    printf("connected PI\n");
}

void connect_DTP() {
    //connect to server DTP port
    if((connect(sock_DTP, (struct sockaddr*)&remote_server_DTP, sockaddr_len)) == ERROR) {
        perror("connect to server DTP port");
        exit(-1);
    }
    printf("connected DTP\n");
}

void send_auth() {
    printf("Authorization required.\n");
    fgets(input, BUFFER, stdin);
    char* quit = strstr(input, "quit");
    send(sock_PI, input, strlen(input), 0);
    int reply_len;
    char auth_reply[BUFFER];
    reply_len = recv(sock_PI, auth_reply, BUFFER, 0);
    if (reply_len) {
        auth_reply[reply_len] = '\0';
        printf("%s\n", auth_reply);
        if(!strstr(auth_reply, "230")) {
            send_auth();
        }
    } else {
        send_auth();
    }
}

void comm_loop() {
    while(true) {
        // fgets() reads input (containing spaces) from user, stores in provided string (input)
        fgets(input, BUFFER, stdin);
        char* quit = strstr(input, "quit");
        send(sock_PI, input, strlen(input), 0);
        // check if the user wants to terminate the program
        if(quit) {
            printf("Connection terminated.\n");
            break;
        }
        len = recv(sock_DTP, output, BUFFER, 0);
        output[len] = '\0';
        printf("%s", output);
    }

    shutdown(sock_PI, SHUT_RDWR);
    shutdown(sock_DTP, SHUT_RDWR);
}

int main(int argc, char *argv[]) {
    init_sockets(argv);
    connect_PI();
    connect_DTP();
    send_auth();
    printf("Ready.\n");
    comm_loop();
    return 0;

}

/* FTP notes
 * Open one port for communication with server and another for file transfer
 * Receiver will convert the data from the standard form to his own internal form.
 *
 */