#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <asm/errno.h>
#include <arpa/inet.h>
#include "server_auth.h"
#include "server_sockets.h"
#include "../JNI/jni_encryption.h"

//TODO adapt for cross-platform uses
//TODO comments
//TODO send large file in chunks

#define MAX_DATA 1024
#define ENCRYPTED_TAG "encrypted_"
#define ENCRYPTED_EXT ".txt"
#define USERS_DIRECTORY "GM_Users"

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

void dir_list(char *cwd) {
    struct dirent *ent;
    char output_path[MAX_DATA];
    char receive[MAX_DATA];
    sprintf(output_path, "%s/%s", cwd, access_path);
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

void list(char* list_type, char * cwd) {
    char receive[MAX_DATA];
    // waits until client data socket is ready to receive data
    int reply_len = recv(client_sock_DTP, receive, MAX_DATA, 0);
    receive[reply_len] = '\0';
    printf("%s\n", receive);
    if (strstr(list_type, "LIST")) {
        char *send_client = "[200] LIST";
        send(client_sock_DTP, send_client, strlen(send_client), 0);
        recv(client_sock_DTP, receive, MAX_DATA, 0);
        dir_list(cwd);
    } else if(strstr(list_type, "HELP")) {
        char *send_client = "[200] HELP\n";
        send(client_sock_DTP, send_client, strlen(send_client), 0);
        recv(client_sock_DTP, receive, MAX_DATA, 0);
        help_list();
    }
}

// get byte array to send to client
char* get_bytes(FILE* f, long numBytes) {
    char *file_bytes = malloc(numBytes);
    fread(file_bytes, 1, numBytes, f);
    return file_bytes;
}

long get_file_size(char* full_path) {
    FILE* f = fopen(full_path, "r");
    fseek(f, 0, SEEK_END);
    long file_len = ftell(f);
    printf("File length: %ld\n", file_len);
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

bool file_available(char* file_name, char *cwd) {
    char search_path[256];
    sprintf(search_path, "%s/%s", cwd, access_path);
    if(is_user_accessible(file_name, search_path)) {
        return true;
    } else {
        printf("File %s does not exist\n", file_name);
        return false;
    }
}

void print_PI_reply() {
    char receive[MAX_DATA];
    int reply_len = recv(client_sock_PI, receive, MAX_DATA, 0);
    receive[reply_len] = '\0';
    printf("%s\n", receive);
}

void send_packets(long num_packets, long packet_size, char *encrypted_path, long last_packet) {
    FILE* f = fopen(encrypted_path, "r");
    fseek(f, 0, SEEK_SET);
    char receive[256];
    for(int i = 0; i < num_packets; i++) {
        char *packet_bytes = get_bytes(f, packet_size);
        send(client_sock_DTP, packet_bytes, packet_size, 0);
        recv(client_sock_DTP, receive, MAX_DATA, 0);
        free(packet_bytes);
    }
    //last packet
    char *packet_bytes = get_bytes(f, last_packet);
    send(client_sock_DTP, packet_bytes, last_packet, 0);
    recv(client_sock_DTP, receive, MAX_DATA, 0);
    free(packet_bytes);
    fclose(f);

}

void split_file(char *encrypted_path) {
    long file_len = get_file_size(encrypted_path);
    char file_len_str[256];
    sprintf(file_len_str, "%ld", file_len);
    send(client_sock_PI, file_len_str, strlen(file_len_str), 0);
    if(file_len <= MAX_DATA) {
        //send as one file
        FILE* f = fopen(encrypted_path, "r");
        char *file_bytes = get_bytes(f, MAX_DATA);
        fclose(f);
        int bytes_sent = send(client_sock_DTP, file_bytes, file_len, 0);
        printf("bytes sent: %d\n", bytes_sent);
        free(file_bytes);
    } else {
        //send in chunks
        long packet_size = MAX_DATA;
        char packet_size_str[256];
        sprintf(packet_size_str, "%ld", packet_size);
        //send client_sock_PI size of chunks
        print_PI_reply();
        send(client_sock_PI, packet_size_str, strlen(packet_size_str), 0);
        print_PI_reply();
        long num_packets = ((file_len - (file_len % packet_size)) / packet_size);
        long last_size = file_len % packet_size;
        char last_size_str[256];
        sprintf(last_size_str, "%ld", last_size);
        //send client size of last packet
        send(client_sock_PI, last_size_str, strlen(last_size_str), 0);
        print_PI_reply();
        //send client_sock_PI number of packets to expect
        char num_packets_str[256];
        sprintf(num_packets_str, "%ld", num_packets);
        send(client_sock_PI, num_packets_str, strlen(num_packets_str), 0);
        //start sending packets over DTP
        send_packets(num_packets, packet_size, encrypted_path, last_size);
        //recv confirmation
        //send chunks via DTP
    }
}

bool send_file(char* args_input, char *cwd) {
    char* file_name = split_args(args_input);
    char receive[MAX_DATA];
    int reply_len;
    char absolute_path[MAX_DATA];
    char encrypted_path[MAX_DATA];
    print_PI_reply();
    if(file_name && file_available(file_name, cwd)) {
        //send file_name to client
        send(client_sock_PI, file_name, strlen(file_name), 0);
        reply_len = recv(client_sock_PI, receive, MAX_DATA, 0);
        receive[reply_len] ='\0';
        printf("%s\n", receive);
        if(strstr(receive, "200")) {
            //send file
            sprintf(absolute_path, "%s/%s/%s", cwd, access_path, file_name);
            if(JNI_encrypt(absolute_path, pass, "encrypt", cwd)) {
                sprintf(encrypted_path, "./%s/%s%s%s", access_path, ENCRYPTED_TAG, file_name, ENCRYPTED_EXT);
            } else {
                printf("%s\n", "Encryption failure.");
                check_log(cwd);
                free(file_name);
                return false;
            }
            split_file(encrypted_path);
            reply_len = recv(client_sock_DTP, receive, MAX_DATA, 0);
            receive[reply_len]='\0';
            printf("%s\n", receive);
            if(strstr(receive, "200")) {
                printf("%s\n", "file transfer success");
            } else {
                printf("%s\n", "file transfer failure");
            }
            // Delete encrypted file
            remove(encrypted_path);
            free(file_name);
            return true;
        } else {
            //dont send file
            char *okay = "200";
            send(client_sock_PI, okay, strlen(okay), 0);
            return false;
        }

    } else {
        char *error = "ERROR: Insufficient arguments or File not found.";
        send(client_sock_PI, error, strlen(error), 0);
        free(file_name);
        return false;
    }

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
    char *port_num_str = split_args(args_input);
    //receive confirmation to delete DTP port
    print_PI_reply();
    long port_num = strtol(port_num_str, NULL, 10);
    if(port_num >= 60000 && port_num <= 65535) {
        //run DTP_port;
        //change to specified port
        char *send_client = "[200] Delete DTP";
        send(client_sock_PI, send_client, strlen(send_client), 0);
        //send client port No.
        print_PI_reply();
        send(client_sock_PI, port_num_str, strlen(port_num_str), 0);
        DTP_port(port_num_str);
        free(port_num_str);
        return test_DTP_connection();
    } else if(port_num == 0) {
        // no number argument
        char *error = "Insufficient arguments.";
        send(client_sock_PI, error, strlen(error), 0);
        free(port_num_str);
        return false;
    } else {
        // number outside of range
        char *error_range = "Port number must be between 60000 and 65535.";
        send(client_sock_PI, error_range, strlen(error_range), 0);
        free(port_num_str);
        return false;
    }

}