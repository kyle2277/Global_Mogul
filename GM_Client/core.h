//
// Created by kylej on 4/5/19.
//

#ifndef GM_CLIENT_CORE_H
#define GM_CLIENT_CORE_H
#define BUFFER 1024
#include <stdio.h>
#include "../JNI/jni_encryption.h"

void sendData(int socket, char *sendStr, int strLen, int flags);
int recvData(int socket, char *recvStr, int recvBuffer, int flags);
void echo_loop();
void serial_recv();
void check_input(char *cwd);
long get_file_len();
void check_output(char *cwd);
void receive_packets(char *file_bytes, long num_packets, long packet_size, long last_packet, FILE* out);
bool can_write(char *file_name, char *full_path);
void file_recv(char *path, char *decrypt_path, char *cwd);
bool dispatch(char *input, char *cwd);
void port();
void test_DTP_connection();

#endif //GM_CLIENT_CORE_H
