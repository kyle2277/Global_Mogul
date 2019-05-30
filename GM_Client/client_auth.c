#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client_auth.h"
#include "client_sockets.h"
#include "core.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
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
    char input[256]; // stores user input
    char auth_reply[BUFFER]; // stores server response
    printf("[332] Authorization required.\nUSER <username>\nPASS <encryption key>\n");
    fgets(input, 256, stdin);
    sendData(sock_PI, input, strlen(input), 0);
    int reply_len;
    reply_len = recvData(sock_PI, auth_reply, BUFFER, 0);
    if (reply_len) {
        auth_reply[reply_len] = '\0';
        if(strstr(input, "PASS") && (strstr(auth_reply, "333") || strstr(auth_reply, "230"))) { set_pass(input); }
        printf("%s\n", auth_reply);
        if(!strstr(auth_reply, "230")) { send_auth(); }
    } else {
        send_auth();
    }
}
