#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include "server_auth.h"
#include "server_sockets.h"
#include "core.h"
#include "../JNI/jni_encryption.h"

// pre-processor definitions
#define ERROR -1
#define MAX_CLIENTS 1
#define MAX_DATA 1024

// TODO add termination function that can exit both client and server at any time

void command_loop() {
    char receive[MAX_DATA];
    bool run = true;
    int data_len;
    while(run) {
        printf("Waiting for client command.\n");
        data_len = recv(client_sock_PI, receive, MAX_DATA, 0);
        receive[data_len] = '\0';
        if(strstr(receive, "QUIT")) {
            char send_client[MAX_DATA] = "Client disconnected.";
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
            send(client_sock_PI, send_client, strlen(send_client), 0);
            printf("%s\n", send_client);
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
        memset(receive, '\0', MAX_DATA);
    }
}

/*
 * UNUSED
*/
void terminate(char* message) {
    perror(message);
    printf("%s\n", "Terminating process.");
    command_loop();
    exit(-1);
}


int main(int argc, char *argv[]) {
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
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
        if(!JNI_init(cwd)) {
            printf("%s\n", "JVM failure.");
            exit(0);
        }
        connect_DTP(sockaddr_len, client_DTP);
        printf("Listening.\n");
        command_loop();
        JNI_end();
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

