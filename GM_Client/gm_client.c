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
    memset(pass, '\n', len);
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
            run = false;
        }
        reply_len = recv(sock_DTP, output, BUFFER, 0);
        output[reply_len] = '\0';
        printf("%s\n", output);
    }
}

void serial_recv() {
    int reply_len;
    char receive[BUFFER]; // stores server response
    char received[BUFFER] = "200"; // confirmation to server when data packet received
    // lets server know data port is ready to receive data
    char data_ready[BUFFER] = "DTP ready";
    send(sock_DTP, data_ready, strlen(data_ready), 0);
    reply_len = recv(sock_DTP, receive, BUFFER, 0);
    do {
        receive[reply_len] = '\0';
        printf("%s\n", receive);
        send(sock_DTP, received, strlen(received), 0);
        reply_len = recv(sock_DTP, receive, BUFFER, 0);
    } while (!strstr(receive, "end"));
    receive[reply_len] = '\0';
    printf("%s\n", receive);
    char confirm_end[BUFFER] = "end received";
    send(sock_DTP, confirm_end, strlen(confirm_end), 0);
}

char* check_input(char* input) {
    int max_args = 2;
    char* delim = " ";
    char* token = strtok(input, delim);
    int token_count = 0;
    // free later
    char* path = malloc(BUFFER);
    bool skip = false;
    while (token != NULL) {
        token_count++;
        token = strtok(NULL, delim);
        if (token_count > max_args) {
            printf("%s\n", "Too many args");
            return NULL;
        } else if (token != NULL) {
            token[(int)strlen(token)-1] = '\0';
            strncpy(path, token, strlen(token));
            return path;
        }
    }
    if (token_count < max_args) {
        printf("%s\n", "Too few args");
        return NULL;
    }
}

void file_recv(char* path) {
    //write to file using a byte array;
    int reply_len;
    char receive[BUFFER];
    // data port ready to receive bytes
    char received[BUFFER] = "200";
    char data_ready[BUFFER] = "DTP ready";
    send(sock_DTP, data_ready, strlen(data_ready), 0);
    reply_len = recv(sock_DTP, receive, BUFFER, 0);
    do {
        receive[reply_len] = '\0';
        printf("%s\n", receive);
        send(sock_DTP, received, strlen(received), 0);
        reply_len = recv(sock_DTP, receive, BUFFER, 0);
    } while(!strstr(receive, "end"));
    receive[reply_len] = '\0';
    printf("%s\n", receive);
    char confirm_end[BUFFER] = "end received";
    send(sock_DTP, confirm_end, strlen(confirm_end), 0);
}

bool dispatch(char* input) {
    if(strstr(input, "ECHO")) {
        echo_loop();
    } else if(strstr(input, "LIST")) {
        serial_recv();
    } else if(strstr(input, "HELP")) {
        serial_recv();
    } else if(strstr(input, "RETR")) {
        char* path = check_input(input);
        if(path) {
            file_recv(path);
        }
        // freeing memory of path variable
        free(path);
    }else if(strstr(input, "QUIT")) {
        return false;
    }
    return true;
}

void command_loop() {
    char input[BUFFER];
    char response[BUFFER];
    int response_len;
    bool run = true;
    while(run) {
        printf("Ready.\n");
        // fgets() reads input (containing spaces) from user, stores in provided string (input)
        fgets(input, BUFFER, stdin);
        char* quit = strstr(input, "QUIT");
        send(sock_PI, input, strlen(input), 0);
        // check if the user wants to terminate the program
        if(quit) {
            printf("Connection terminated.\n");
            run = false;
        }
        response_len = recv(sock_PI, response, BUFFER, 0);
        response[response_len] = '\0';
        printf("%s\n", response);
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