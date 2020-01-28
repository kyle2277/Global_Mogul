//
// Created by kylej on 4/5/19.
//

#ifndef GM_SERVER_CORE_H
#define GM_SERVER_CORE_H
#define MAX_DATA 1024

#include <stdio.h>
#include "../JNI/jni_encryption.h"
//change location of these variables
struct sockaddr_in server_PI;
struct sockaddr_in server_DTP;
struct sockaddr_in client_PI;
struct sockaddr_in client_DTP;

void echo_loop();
void dir_list(char *cwd);
void help_list();
void list(char *list_type, char *cwd);
char* get_bytes(FILE* f, long numBytes);
long get_file_size(char *path);
char* split_args(char *receive);
bool file_available(char *path, char *cwd);
void print_PI_reply();
void send_packets(long num_packets, long packet_size, char *encrypted_path, long last_packet);
void split_file(char *encrypted_path);
bool send_file(char *args_input, char *cwd);
bool port(char *args_input);
bool test_DTP_connection();

#endif //GM_SERVER_CORE_H
