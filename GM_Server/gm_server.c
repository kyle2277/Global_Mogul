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
            if(quit) {
                char response[MAX_DATA] = "Exiting ECHO";
                printf("%s\n", response);
                send(client_sock_DTP, response, strlen(response), 0);
                break;
            }
            send(client_sock_DTP, data, data_len, 0);
            printf("Sent mesg: %s", data);
        }
    }
}

void dir_list() {
    struct dirent *ent;
    char path[MAX_DATA];
    char receive[MAX_DATA];
    sprintf(path, "%s%s", "./", access_path);
    printf("%s\n", path);
    DIR *dir = opendir(path);
    while ((ent = readdir(dir)) != NULL) {
        char name[MAX_DATA];
        strncpy(name, ent->d_name, strlen(ent->d_name));
        name[strlen(ent->d_name)] = '\0';
        printf("%s\n", name);
        send(client_sock_DTP, name, strlen(name), 0);
        recv(client_sock_DTP, receive, MAX_DATA, 0);
        if(!strstr(receive, "200")) {
            printf("%s\n", "error");
            break;
        }
    }
    char end[MAX_DATA] = "end";
    send(client_sock_DTP, end, strlen(end), 0);
    recv(client_sock_DTP, receive, MAX_DATA, 0);
    printf("%s\n", receive);
    closedir(dir);
}

void help_list() {
    char receive[MAX_DATA];
    char send_client[MAX_DATA] = "this is the list";
    send(client_sock_DTP, send_client, strlen(send_client), 0);
    recv(client_sock_DTP, receive, MAX_DATA, 0);
    char end[MAX_DATA] = "end";
    send(client_sock_DTP, end, strlen(end), 0);
    recv(client_sock_DTP, receive, MAX_DATA, 0);
    printf("%s\n", receive);
}

void list(char* list_type) {
    char receive[MAX_DATA];
    // waits until client data socket is ready to receive data
    int reply_len = recv(client_sock_DTP, receive, MAX_DATA, 0);
    receive[reply_len] = '\0';
    printf("%s\n", receive);
    if (strstr(list_type, "LIST")) {
        char send_client[MAX_DATA] = "[200] LIST";
        send(client_sock_DTP, send_client, strlen(send_client), 0);
        recv(client_sock_DTP, receive, MAX_DATA, 0);
        dir_list();
    } else if(strstr(list_type, "HELP")) {
        char send_client[MAX_DATA] = "[200] HELP";
        send(client_sock_DTP, send_client, strlen(send_client), 0);
        recv(client_sock_DTP, receive, MAX_DATA, 0);
        help_list();
    }
}

// get byte array to send to client
char* get_bytes(char* path) {
    char full_path[MAX_DATA];
    sprintf(full_path, "./user/%s", path);
    FILE* f = fopen(full_path, "r");
    fseek(f, 0, SEEK_END);
    long file_len = ftell(f);
    char* file_bytes = malloc(file_len);
    fseek(f, 0, SEEK_SET);
    fread(file_bytes, 1, file_len, f);
    fclose(f);
    return file_bytes;
}

long get_file_size(char* path) {
    char full_path[MAX_DATA];
    sprintf(full_path, "./user/%s", path);
    FILE* f = fopen(full_path, "r");
    fseek(f, 0, SEEK_END);
    long file_len = ftell(f);
    fclose(f);
    return file_len;
}

char* split_args(char* receive) {
    int max_args = 2;
    char* delim = " ";
    char* token = strtok(receive, delim);
    int token_count = 0;
    // free later
    char* path = malloc(MAX_DATA);
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

bool file_available(char* path) {
    char full_path[MAX_DATA];
    sprintf(full_path, "./user/%s", path);
    FILE* f;
    if(f = fopen(full_path, "r")) {
        fclose(f);
        return true;
    }
    printf("File %s does not exist", path);
    return false;
}

char* print_reply(char* receive) {
    int reply_len = recv(client_sock_PI, receive, MAX_DATA, 0);
    receive[reply_len] = '\0';
    printf("%s\n", receive);
    return receive;
}

int send_file(char* args_input) {
    char* path = split_args(args_input);
    char receive[MAX_DATA];
    int reply_len;
    if(path && file_available(path)) {
        print_reply(receive);
        char success[MAX_DATA] = "200";
        send(client_sock_PI, success, strlen(success), 0);
        reply_len = recv(client_sock_PI, receive, MAX_DATA, 0);
        receive[reply_len] = '\0';
        //send path to client
        send(client_sock_PI, path, strlen(path), 0);
    } else {
        char error[MAX_DATA] = "Too many or too few args";
        send(client_sock_PI, error, strlen(error), 0);
        return 0;
    }
    //client asks for file length
    print_reply(receive);
    long file_len = get_file_size(path);
    send(client_sock_PI, &file_len, file_len, 0);
    char* file_bytes = get_bytes(path);
    send(client_sock_DTP, file_bytes, file_len, 0);
    reply_len = recv(client_sock_DTP, receive, MAX_DATA, 0);
    receive[reply_len] = '\0';
    printf("%s\n", receive);
    if(strstr(receive, "200")) {
        printf("%s\n", "file transfer success");
    } else {
        printf("%s\n", "file transfer failure");
    }
    return 1;
}

//todo write cases for specific commands
void command_loop() {
    char receive[MAX_DATA];
    bool run = true;
    int data_len;
    while(run) {
        printf("Waiting for client command.\n");
        data_len = recv(client_sock_PI, receive, MAX_DATA, 0);
        receive[data_len] = '\0';
        if(strstr(receive, "QUIT")) {
            char send_client[MAX_DATA] = "Client disconnected";
            send(client_sock_PI, send_client, strlen(send_client), 0);
            printf("%s\n", send_client);
            clean("Username", access_path);
            clean("Password", pass);
            shutdown(client_sock_PI, SHUT_RDWR);
            shutdown(client_sock_DTP, SHUT_RDWR);
            run = false;
        } else if(strstr(receive, "ECHO")) {
            char send_client[MAX_DATA] = "Entering ECHO";
            printf("%s\n", send_client);
            send(client_sock_PI, send_client, strlen(send_client) , 0);
            echo_loop();
        } else if(strstr(receive, "LIST")) {
            char send_client[MAX_DATA] = "Sending list";
            printf("%s\n", send_client);
            send(client_sock_PI, send_client, strlen(send_client), 0);
            // list available files
            list("LIST");
        } else if(strstr(receive, "HELP")) {
            char send_client[MAX_DATA] = "Help list";
            printf("%s\n", send_client);
            send(client_sock_PI, send_client, strlen(send_client), 0);
            // help list
            list("HELP");
        } else if(strstr(receive, "RETR")) {
            char send_client[MAX_DATA] = "Retrieve";
            printf("%s\n", send_client);
            send(client_sock_PI, send_client, strlen(send_client), 0);
            // send file
            send_file(receive);
        } else if(strstr(receive, "NOOP")) {
                char send_client[MAX_DATA] = "[200] command OK";
                printf("%s\n", send_client);
                send(client_sock_PI, send_client, strlen(send_client), 0);
        } else {
            char send_client[MAX_DATA] = "[500] syntax error, command unrecognized";
            printf("%s\n", send_client);
            send(client_sock_PI, send_client, strlen(send_client), 0);
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

