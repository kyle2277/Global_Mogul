//
// Created by kylej on 4/5/19.
//

#ifndef GM_SERVER_CORE_H
#define GM_SERVER_CORE_H
#define MAX_DATA 1024

typedef enum { false, true } bool;
void terminate(char* message);
void echo_loop();
void dir_list();
void help_list();
void list(char* list_type);
char* get_bytes(char* path);
long get_file_size(char* path);
char* split_args(char* receive);
bool file_available(char* path);
char* print_reply(char* receive);
int send_file(char* args_input);


#endif //GM_SERVER_CORE_H
