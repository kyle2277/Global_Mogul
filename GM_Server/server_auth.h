//
// Created by kylej on 3/19/19.
//

#ifndef GM_SERVER_AUTHORIZE_H
#define GM_SERVER_AUTHORIZE_H
#define MAX_DATA 1024

#include "../JNI/jni_encryption.h"

// name of user
char access_path[MAX_DATA];
// encryption key
char pass[MAX_DATA];

void clean(char *type, char *cred);
bool is_user_accessible(char *args, char *search_path);
void submit_auth(char *args[], char *cwd);
void get_auth(char *cwd);

#endif //GM_SERVER_AUTHORIZE_H
