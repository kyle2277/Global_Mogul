#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include "server_auth.h"
#include "server_sockets.h"

// pre-processor definitions
#define MAX_DATA 1024
#define USERS_DIRECTORY "GM_Users"

/*
 * Replaces all characters in current credentials with the null string character. Prepares system for new user
 */
void clean(char *type, char *cred) {
    int len = strlen(cred);
    memset(cred, '\0', len);
    printf("%s cleared.\n", type);
}

/*
 * Check if user directory exists
 */
bool is_user_accessible(char *args, char *search_path) {
    struct dirent *ent;
    char compare[256];
    sprintf(compare, "%s", args);
    DIR *dir = opendir(search_path);
    while ((ent = readdir(dir)) != NULL) {
        char name[256];
        strncpy(name, ent->d_name, strlen(ent->d_name));
        name[strlen(ent->d_name)] = '\0';
        if(strcmp(name, compare) == 0) {
            return true;
        }
    }
    return false;
}

/*
 * Stores username and password from client input
 */
void submit_auth(char *args[], char *cwd) {
    if(strstr(args[0], "USER")) {
        if(access_path[0] != '\0') { clean("Username", access_path); }
        char user_path[256];
        sprintf(user_path, "%s/%s", cwd, USERS_DIRECTORY);
        if(is_user_accessible(args[1], user_path)) {
            sprintf(access_path, "%s/%s", USERS_DIRECTORY, args[1]);
            printf("%s\n", access_path);
            if(access_path[0] != '\0' && pass[0] != '\0') {
                // 230: user logged in, proceed
                char* response = "[230] login successful";
                send(client_sock_PI, response, strlen(response), 0);
            } else {
                char* response = "[331] Username okay, need password";
                send(client_sock_PI, response, strlen(response), 0);
            }
        } else {
            char response[256];
            sprintf(response, "[530] User %s does not exist", args[1]);
            send(client_sock_PI, response, strlen(response), 0);
            clean("Username", access_path);
        }
    } else if(strstr(args[0], "PASS")) {
        if(pass[0] != '\0') { clean("Password", pass); }
        strncpy(pass, args[1], strlen(args[1]));
        printf("%s\n", pass);
        if(access_path[0] != '\0' && pass[0] != '\0') {
            // 230: user logged in, proceed
            char* response = "[230] login successful";
            send(client_sock_PI, response, strlen(response), 0);
        } else {
            char* response = "[333] Password okay, need username";
            send(client_sock_PI, response, strlen(response), 0);
        }
    } else {
        char* response = "[500] Syntax error";
        send(client_sock_PI, response, strlen(response), 0);
    }

}

/*
 * Facilitates authorization of a newly connected user. Username and password required
 */
void get_auth(char *cwd) {
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
                    char *send_client = "[500] Too many args";
                    send(client_sock_PI, send_client, strlen(send_client), 0);
                    clean("Username", access_path);
                    clean("Password", pass);
                    skip = true;
                    break;
                }
                if (token != NULL) {
                    token[strlen(token)-1] = '\0';
                    args[token_count] = token;
                }
            }
            if(!skip) {
                if (token_count < max_args) {
                    char *send_client = "[500] Too few args";
                    send(client_sock_PI, send_client, strlen(send_client), 0);
                    clean("Username", access_path);
                    clean("Password", pass);
                    get_auth(cwd);
                } else {
                    submit_auth(args, cwd);
                }
            }
        }
    } while (access_path[0] == '\0' || pass[0] == '\0');
}