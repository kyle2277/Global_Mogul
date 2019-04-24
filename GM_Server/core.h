//
// Created by kylej on 4/5/19.
//

#ifndef GM_SERVER_CORE_H
#define GM_SERVER_CORE_H
#define MAX_DATA 1024

#include "../JNI/jni_encryption.h"
//change location of these varibles
struct sockaddr_in server_PI;
struct sockaddr_in server_DTP;
struct sockaddr_in client_PI;
struct sockaddr_in client_DTP;

void echo_loop();
void dir_list();
void help_list();
void list(char *list_type);
char* get_bytes(char *path);
long get_file_size(char *path);
char* split_args(char *receive);
bool file_available(char *path);
void print_reply(char *receive);
bool send_file(char *args_input, char *cwd);
bool port(char *args_input);
bool test_DTP_connection();

#endif //GM_SERVER_CORE_H
