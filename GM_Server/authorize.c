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
#define MAX_DATA 1024

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