#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include "server_auth.h"
#include "server_sockets.h"
#include "../JNI/jni_encryption.h"

//TODO adapt for cross-platform uses
//TODO comments
//TODO change 'print_reply' to 'print_PI_reply'

#define MAX_DATA 1024
#define ENCRYPTED_TAG "encrytped_"
#define ENCRYPTED_EXT ".txt"

 /*
 * receives data from client on PI channel and echoes data back on DTP channel.
 * DEPRECATED
 */
void echo_loop() {
    char data[MAX_DATA];
    int data_len = 1;
    // loop while client is connected to the server port and authorized
    while(data_len) {
        // wait for data from the client
        data_len = recv(client_sock_PI, data, MAX_DATA, 0);
        if(data_len) {
            data[data_len] = '\0';
            char* quit = strstr(data, "QUIT");
            // check if the user has terminated the client-side of the program
            if(quit) {
                char *response = "Exiting ECHO";
                printf("%s\n", response);
                send(client_sock_DTP, response, strlen(response), 0);
                break;
            }
            send(client_sock_DTP, data, data_len, 0);
            printf("Sent mesg: %s", data);
        }
    }
}

void dir_list() {
    struct dirent *ent;
    char output_path[MAX_DATA];
    char receive[MAX_DATA];
    sprintf(output_path, "%s%s", "./", access_path);
    printf("%s\n", output_path);
    DIR *dir = opendir(output_path);
    while ((ent = readdir(dir)) != NULL) {
        char name[256];
        strncpy(name, ent->d_name, strlen(ent->d_name));
        name[strlen(ent->d_name)] = '\0';
        printf("%s\n", name);
        send(client_sock_DTP, name, strlen(name), 0);
        recv(client_sock_DTP, receive, MAX_DATA, 0);
        if(!strstr(receive, "200")) {
            printf("%s\n", "error");
            break;
        }
    }
    char *end = "end";
    send(client_sock_DTP, end, strlen(end), 0);
    int len = recv(client_sock_DTP, receive, MAX_DATA, 0);
    receive[len] = '\0';
    printf("%s\n", receive);
    closedir(dir);
}

void help_list() {
    char receive[MAX_DATA];
    char *send_client = "Commands:\nECHO - echoes input\nLIST - lists retrievable files\n"
                         "RETR <file name> - retrieve file\nNOOP - check connection\nHELP - help list\nQUIT - exit";
    send(client_sock_DTP, send_client, strlen(send_client), 0);
    recv(client_sock_DTP, receive, MAX_DATA, 0);
    char *end = "end";
    send(client_sock_DTP, end, strlen(end), 0);
    int len = recv(client_sock_DTP, receive, MAX_DATA, 0);
    receive[len] = '\0';
    printf("%s\n", receive);
}

void list(char* list_type) {
    char receive[MAX_DATA];
    // waits until client data socket is ready to receive data
    int reply_len = recv(client_sock_DTP, receive, MAX_DATA, 0);
    receive[reply_len] = '\0';
    printf("%s\n", receive);
    if (strstr(list_type, "LIST")) {
        char *send_client = "[200] LIST";
        send(client_sock_DTP, send_client, strlen(send_client), 0);
        recv(client_sock_DTP, receive, MAX_DATA, 0);
        dir_list();
    } else if(strstr(list_type, "HELP")) {
        char *send_client = "[200] HELP\n";
        send(client_sock_DTP, send_client, strlen(send_client), 0);
        recv(client_sock_DTP, receive, MAX_DATA, 0);
        help_list();
    }
}

// get byte array to send to client
char* get_bytes(char* full_path) {
    FILE* f = fopen(full_path, "r");
    fseek(f, 0, SEEK_END);
    long file_len = ftell(f);
    char* file_bytes = malloc(file_len);
    fseek(f, 0, SEEK_SET);
    fread(file_bytes, 1, file_len, f);
    fclose(f);
    return file_bytes;
}

long get_file_size(char* full_path) {
    FILE* f = fopen(full_path, "r");
    fseek(f, 0, SEEK_END);
    long file_len = ftell(f);
    fclose(f);
    return file_len;
}

char* split_args(char* receive) {
    int max_args = 2;
    char* delim = " ";
    char* token = strtok(receive, delim);
    int token_count = 0;
    // free later
    char* argument = malloc(256);
    while (token != NULL) {
        token_count++;
        token = strtok(NULL, delim);
        if (token_count > max_args) {
            printf("%s\n", "Too many args");
            return NULL;
        } else if (token != NULL) {
            strncpy(argument, token, strlen(token));
            argument[strlen(token)-1] = '\0';
            return argument;
        }
    }
    if (token_count < max_args) {
        printf("%s\n", "Too few args");
        return NULL;
    }
    return NULL;
}

bool file_available(char* file_name) {
    char full_path[MAX_DATA];
    sprintf(full_path, "./%s/%s", access_path, file_name);
    FILE* f;
    if((f = fopen(full_path, "r"))) {
        fclose(f);
        return true;
    }
    printf("File %s does not exist\n", file_name);
    return false;
}

void print_reply(char* receive) {
    int reply_len = recv(client_sock_PI, receive, MAX_DATA, 0);
    receive[reply_len] = '\0';
    printf("%s\n", receive);
}

bool send_file(char* args_input, char *cwd) {
    char* file_name = split_args(args_input);
    char receive[MAX_DATA];
    int reply_len;
    char absolute_path[MAX_DATA];
    char encrypted_path[MAX_DATA];
    print_reply(receive);
    if(file_name && file_available(file_name)) {
        sprintf(absolute_path, "%s/%s/%s", cwd, access_path, file_name);
        if(JNI_encrypt(absolute_path, pass, "encrypt", cwd)) {
            sprintf(encrypted_path, "./%s/%s%s%s", access_path, ENCRYPTED_TAG, file_name, ENCRYPTED_EXT);
            char *success = "200";
            send(client_sock_PI, success, strlen(success), 0);
            reply_len = recv(client_sock_PI, receive, MAX_DATA, 0);
            receive[reply_len] = '\0';
            //send file_name to client
            send(client_sock_PI, file_name, strlen(file_name), 0);
        } else {
            printf("%s\n", "Encryption failure.");
            check_log(cwd);
            free(file_name);
            return false;
        }
    } else {
        char *error = "Insufficient arguments or File not found.";
        send(client_sock_PI, error, strlen(error), 0);
        free(file_name);
        return false;
    }
    //client asks for file length
    print_reply(receive);
    long file_len = get_file_size(encrypted_path);
    send(client_sock_PI, &file_len, file_len, 0);
    char* file_bytes = get_bytes(encrypted_path);
    send(client_sock_DTP, file_bytes, file_len, 0);
    reply_len = recv(client_sock_DTP, receive, MAX_DATA, 0);
    receive[reply_len] = '\0';
    printf("%s\n", receive);
    if(strstr(receive, "200")) {
        printf("%s\n", "file transfer success");
    } else {
        printf("%s\n", "file transfer failure");
    }
    // Delete encrypted file
    remove(encrypted_path);
    free(file_name);
    free(file_bytes);
    return true;
}

bool test_DTP_connection() {
    char receive[MAX_DATA];
    int len = recv(client_sock_DTP, receive, MAX_DATA, 0);
    receive[len] = '\0';
    printf("%s\n", receive);
    if(strstr(receive, "200")) {
        char *send_client = "200";
        send(client_sock_DTP, send_client, strlen(send_client), 0);
        return true;
    } else {
        return false;
    }
}

bool port(char *args_input) {
    //PORT INPUT MUST BE INT
    char *port_num = split_args(args_input);
    //receive confirmation to delete DTP port
    char receive[MAX_DATA];
    print_reply(receive);
    if((int) port_num && (int) port_num >= 60000 && (int) port_num <= 65535) {
        //change to specified port
        char *send_client = "[200] Delete DTP";
        send(client_sock_PI, send_client, strlen(send_client), 0);
        //send client port No.
        print_reply(receive);
        strncpy(send_client, port_num, strlen(port_num));
        send(client_sock_PI, send_client, strlen(send_client), 0);
        DTP_port(port_num);
        return test_DTP_connection();
    } else if ((int) port_num && ((int)port_num < 60000) && ((int)port_num > 65535)){
        //port number invalid must be between 60000 and 65536
    } else { //port does not exist
        char *error = "Insufficient arguments.";
        send(client_sock_PI, error, strlen(error), 0);
        free(port_num);
        return false;
    }
}