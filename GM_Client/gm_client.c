#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "client_sockets.h"
#include "client_auth.h"
#include "core.h"

#define ERROR -1
#define BUFFER 1024
#define DEFAULT_PORT "60000"

//TODO argv[0] is the current cmake directory
//todo clean up global variables

void command_loop(char *cwd) {
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
        run = dispatch(input, cwd);
        memset(input, '\0', BUFFER);
    }
    clean_pass();
    shutdown(sock_PI, SHUT_RDWR);
    shutdown(sock_DTP, SHUT_RDWR);
}

int main(int argc, char *argv[]) {
    set_server_addr(argv[1]);
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    struct sockaddr_in remote_server_PI;
    struct sockaddr_in remote_server_DTP;
    int sockaddr_len = sizeof(struct sockaddr_in);
    init_PI_socket(remote_server_PI);
    init_DTP_socket(remote_server_DTP, DEFAULT_PORT);

    connect_PI(sockaddr_len, remote_server_PI);
    send_auth();
    if(!JNI_init(cwd)) {
        printf("%s\n", "JVM failure.");
        exit(0);
    }
    connect_DTP(sockaddr_len, remote_server_DTP);
    command_loop(cwd);
    JNI_end();
    return 0;
}

/* FTP notes
 * Open one port for communication with server and another for file transfer
 * Receiver will convert the data from the standard form to his own internal form.
 *
 */