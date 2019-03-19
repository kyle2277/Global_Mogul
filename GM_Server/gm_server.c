#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <dirent.h>

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

/*
 * Initialization of all socket descriptors and structures
 */
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

/*
 * Replaces all characters in current credentials with the null string character. Prepares system for new user
 */
void clean(char type[], char cred[]) {
    int len = strlen(cred);
    for(int i = 0; i < len; i++) { cred[i] = '\0'; }
    printf("%s cleared.\n", type);
}

/*
 * Check if user directory exists
 */
bool is_valid_user(char args[]) {
    bool valid = false;
    DIR* directory = opendir(args);
    if(ENOENT != errno) { valid = true; }
    closedir(directory);
    return valid;
}

/*
 * Stores username and password from client input
 */
void submit_auth(char* args[]) {
    if(strstr(args[0], "USER")) {
        if(access_path[0] != '\0') { clean("Username", access_path); }
        if(is_valid_user(args[1])) {
            strncpy(access_path, args[1], strlen(args[1]));
            printf("%s\n", access_path);
            if(access_path[0] != '\0' && pass[0] != '\0') {
                // 230: user logged in, proceed
                send(client_sock_PI, "[230] login successful", MAX_DATA, 0);
            } else {
                send(client_sock_PI, "[330] User name okay, need password", MAX_DATA, 0);
            }
        } else {
            char reply[MAX_DATA];
            sprintf(reply, "[530] User %s does not exist", args[1]);
            send(client_sock_PI, reply, MAX_DATA, 0);
            clean("Username", access_path);
        }
    } else if(strstr(args[0], "PASS")) {
        if(pass[0] != '\0') { clean("Password", pass); }
        strncpy(pass, args[1], strlen(args[1]));
        printf("%s\n", pass);
        if(access_path[0] != '\0' && pass[0] != '\0') {
            // 230: user logged in, proceed
            send(client_sock_PI, "[230] login successful", MAX_DATA, 0);
        } else {
            send(client_sock_PI, "Password received", MAX_DATA, 0);
        }
    } else {
        send(client_sock_PI, "[500] Syntax error", MAX_DATA, 0);
    }
}

/*
 * Facilitates authorization of a newly connected user. Username and password required
 */
void get_auth() {
    do {
        int max_args = 2;
        char *delim = " ";
        char *args[max_args];
        char usr_input[MAX_DATA];
        int auth_len;
        bool skip = false;
        auth_len = recv(client_sock_PI, usr_input, MAX_DATA, 0);
        // if anything is received form the client, proceed with authorization
        if (auth_len) {
            usr_input[auth_len] = '\0';
            char *token = strtok(usr_input, delim);
            int token_count = 0;
            args[token_count] = token;
            while (token != NULL) {
                token_count++;
                token = strtok(NULL, delim);
                if (token_count > max_args) {
                    send(client_sock_PI, "[500] Too many args", MAX_DATA, 0);
                    clean("Username", access_path);
                    clean("Password", pass);
                    skip = true;
                    break;
                }
                if (token != NULL) {
                    token[(int)strlen(token)-1] = '\0';
                    args[token_count] = token;
                }
            }
            if(!skip) {
                if (token_count < max_args) {
                    send(client_sock_PI, "[500] Too few args", MAX_DATA, 0);
                    clean("Username", access_path);
                    clean("Password", pass);
                    get_auth();
                } else {
                    submit_auth(args);
                }
            }
        }
    } while (access_path[0] == '\0' || pass[0] == '\0');
}

/*
 * receives data from client on PI channel and echoes data back on DTP channel.
 */
void echo_loop() {
    data_len = 1;
    // loop while client is connected to the server port and authorized
    while(data_len) {
        // wait for data from the client
        data_len = recv(client_sock_PI, data, MAX_DATA, 0);
        if(data_len) {
            char* quit = strstr(data, "quit");
            // check if the user has terminated the client-side of the program
            if(quit) { break; }
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
        connect_PI();
        get_auth();
        connect_DTP();
        printf("Listening.\n");
        echo_loop();
        printf("Client disconnected.\n");
        clean("Username", access_path);
        clean("Password", pass);
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

