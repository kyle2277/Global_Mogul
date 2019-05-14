#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include "server_auth.h"
#include "server_sockets.h"
#include "core.h"

// pre-processor definitions
#define ERROR -1
#define MAX_CLIENTS 1
#define MAX_DATA 1024
#define DEFAULT_PORT "60000"

void command_loop(char *cwd) {
    char receive[MAX_DATA];
    bool run = true;
    int data_len;
    while(run) {
        printf("Waiting for client command.\n");
        data_len = recv(client_sock_PI, receive, MAX_DATA, 0);
        receive[data_len] = '\0';
        if(strstr(receive, "QUIT")) {
            char *send_client = "Client disconnected.";
            send(client_sock_PI, send_client, strlen(send_client), 0);
            printf("%s\n", send_client);
            clean("Username", access_path);
            clean("Password", pass);
            shutdown(client_sock_PI, SHUT_RDWR);
            shutdown(client_sock_DTP, SHUT_RDWR);
            run = false;
        } else if(strstr(receive, "ECHO")) {
            char *send_client = "Entering ECHO";
            printf("%s\n", send_client);
            send(client_sock_PI, send_client, strlen(send_client) , 0);
            echo_loop();
        } else if(strstr(receive, "LIST")) {
            char *send_client = "Sending list";
            printf("%s\n", send_client);
            send(client_sock_PI, send_client, strlen(send_client), 0);
            // list available files
            list("LIST", cwd);
        } else if(strstr(receive, "HELP")) {
            char *send_client = "Help list";
            printf("%s\n", send_client);
            send(client_sock_PI, send_client, strlen(send_client), 0);
            // help list
            list("HELP", cwd);
        } else if(strstr(receive, "RETR")) {
            char *send_client = "Retrieve";
            send(client_sock_PI, send_client, strlen(send_client), 0);
            printf("%s\n", send_client);
            // send file
            send_file(receive, cwd);
        } else if(strstr(receive, "PORT")) {
            // set data transfer port to the specified number
            // must be greater than 60000 and less than 65536
            char *send_client = "Set Data Transfer Process port";
            send(client_sock_PI, send_client, strlen(send_client), 0);
            printf("%s\n", send_client);
            // switch port
            if(port(receive)) {
                //port assignment success
                printf("%s\n", "Port assignment success.");
            } else {
                // port assignment failure
                printf("%s\n", "Port assignment failure.");
            }
        } else if(strstr(receive, "NOOP")) {
                char *send_client = "[200] command OK";
                printf("%s\n", send_client);
                send(client_sock_PI, send_client, strlen(send_client), 0);
        } else {
            char *send_client = "[500] syntax error, command unrecognized";
            printf("%s\n", send_client);
            send(client_sock_PI, send_client, strlen(send_client), 0);
        }
        memset(receive, '\0', MAX_DATA);
    }
}

int main(int argc, char *argv[]) {
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    sockaddr_len = sizeof(struct sockaddr_in);
    if(!JNI_init(cwd)) {
        printf("%s\n", "JVM failure.");
        exit(0);
    }
    init_PI_socket();
    init_DTP_socket(DEFAULT_PORT);
    listen_PI();
    listen_DTP();

    while(true) {
        printf("%s\n", "Waiting for client connection ...");
        connect_PI();
        get_auth(cwd);
        connect_DTP();
        printf("Listening.\n");
        command_loop(cwd);
        //JNI_end();
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

