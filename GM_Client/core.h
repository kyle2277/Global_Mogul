//
// Created by kylej on 4/5/19.
//

#ifndef GM_CLIENT_CORE_H
#define GM_CLIENT_CORE_H
#define BUFFER 1024

#include "../JNI/jni_encryption.h"

void echo_loop();
void serial_recv();
char* check_input(char* input);
int get_file_len();
void check_output();
bool can_write(char* file_name, char* full_path);
void file_recv(char* path);
bool dispatch(char* input);

#endif //GM_CLIENT_CORE_H
