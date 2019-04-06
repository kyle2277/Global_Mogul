#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "core.h"
#include "client_sockets.h"

/* DEPRECATED */
void echo_loop() {
    int reply_len;
    char input[BUFFER]; // stores user input
    char output[BUFFER]; // stores remote_server response
    bool run = true;
    while(run) {
        // fgets() reads input (containing spaces) from user, stores in provided string (input)
        fgets(input, BUFFER, stdin);
        char* quit = strstr(input, "QUIT");
        send(sock_PI, input, strlen(input), 0);
        // check if the user wants to terminate the program
        if(quit) {
            run = false;
        }
        reply_len = recv(sock_DTP, output, BUFFER, 0);
        output[reply_len] = '\0';
        printf("%s\n", output);
    }
}

void serial_recv() {
    int reply_len;
    char receive[BUFFER]; // stores server response
    char received[BUFFER] = "200"; // confirmation to server when data packet received
    // lets server know data port is ready to receive data
    char data_ready[BUFFER] = "DTP ready";
    send(sock_DTP, data_ready, strlen(data_ready), 0);
    reply_len = recv(sock_DTP, receive, BUFFER, 0);
    do {
        receive[reply_len] = '\0';
        printf("%s\n", receive);
        send(sock_DTP, received, strlen(received), 0);
        reply_len = recv(sock_DTP, receive, BUFFER, 0);
    } while (!strstr(receive, "end"));
    receive[reply_len] = '\0';
    printf("%s\n", receive);
    char confirm_end[BUFFER] = "end received";
    send(sock_DTP, confirm_end, strlen(confirm_end), 0);
}

char* check_input(char* input) {
    char send_server[BUFFER] = "args ok?";
    char* receive = malloc(BUFFER);
    send(sock_PI, send_server, strlen(send_server), 0);
    int reply_len = recv(sock_PI, receive, BUFFER, 0);
    receive[reply_len] = '\0';
    if (strstr(receive, "200")) {
        //success, get path
        strncpy(send_server, "path", strlen("path"));
        send(sock_PI, send_server, strlen(send_server), 0);
        reply_len = recv(sock_PI, receive, BUFFER, 0);
        receive[reply_len] = '\0';
        printf("%s\n", receive);
        return receive;
    } else {
        printf("%s\n", receive);
        return NULL;
        //failure
    }
}

int get_file_len() {
    char length[BUFFER];
    char data_ready[BUFFER] = "File length";
    send(sock_PI, data_ready, strlen(data_ready), 0);
    int reply_len = recv(sock_PI, length, BUFFER, 0);
    length[reply_len] = '\0';
    return (int) *length;
}

void file_recv(char* path) {

    // write to file using a byte array;
    // data port ready to receive bytes
    int file_len = get_file_len();
    char* file_bytes = malloc(file_len);
    char data_ready[BUFFER] = "DTP ready";
    send(sock_DTP, data_ready, strlen(data_ready), 0);
    recv(sock_DTP, file_bytes, file_len, 0);
    char full_path[BUFFER];
    sprintf(full_path, "./%s", path);
    FILE* out = fopen(full_path, "w");
    fwrite(file_bytes, 1, file_len, out);
    fclose(out);
    char confirm_end[BUFFER] = "[200] end received";
    send(sock_DTP, confirm_end, strlen(confirm_end), 0);
    printf("%s\n", confirm_end);
}

bool dispatch(char* input) {
    if(strstr(input, "ECHO")) {
        echo_loop();
    } else if(strstr(input, "LIST")) {
        serial_recv();
    } else if(strstr(input, "HELP")) {
        serial_recv();
    } else if(strstr(input, "RETR")) {
        char* path = check_input(input);
        if(path) {
            file_recv(path);
        }
        // freeing memory of path variable
        free(path);
    }else if(strstr(input, "QUIT")) {
        return false;
    }
    return true;
}