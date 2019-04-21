#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "client_sockets.h"
#include "client_auth.h"
#include "core.h"
#include "../JNI/jni_encryption.h"

#define ERROR -1
#define BUFFER 1024

//todo clean up global variables

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
        memset(input, '\0', BUFFER);
    }
    clean_pass();
    shutdown(sock_PI, SHUT_RDWR);
    shutdown(sock_DTP, SHUT_RDWR);
}

int main(int argc, char *argv[]) {
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    int sockaddr_len = sizeof(struct sockaddr_in);
    init_sockets(argv);

    connect_PI(sockaddr_len);
    send_auth();
    if(!JNI_init(cwd)) {
        printf("%s\n", "JVM failure.");
        exit(0);
    }
    connect_DTP(sockaddr_len);
    command_loop();
    JNI_end();
    return 0;
}

/* FTP notes
 * Open one port for communication with server and another for file transfer
 * Receiver will convert the data from the standard form to his own internal form.
 *
 */