//
// Created by kylej on 3/19/19.
//

#ifndef GM_SERVER_AUTHORIZE_H
#define GM_SERVER_AUTHORIZE_H
#define MAX_DATA 1024

#include "../JNI/jni_encryption.h"

int sock_PI; // reference to the server's Protocol Interpreter (PI) socket which talks to the client
int sock_DTP; // reference to the server's Data Transfer Process (DTP) socket which exchanges data with the client
int client_sock_PI; // reference to connected client PI socket
int client_sock_DTP; // reference to connected client DTP socket
// name of user
char access_path[MAX_DATA];
// encryption key
char pass[MAX_DATA];

void clean(char type[], char cred[]);
bool is_valid_user(char args[]);
void submit_auth(char* args[]);
void get_auth();

#endif //GM_SERVER_AUTHORIZE_H
