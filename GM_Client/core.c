#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../JNI/jni_encryption.h"
#include "client_sockets.h"
#include "client_auth.h"

#define OUTPUT "output"
#define ENCRYPTED_TAG "encrypted_"
#define ENCRYPTED_EXT ".txt"

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
    char confirm_end[BUFFER] = "end received";
    send(sock_DTP, confirm_end, strlen(confirm_end), 0);
}

char* check_input() {
    char send_server[BUFFER] = "args ok?";
    char* receive = malloc(BUFFER);
    send(sock_PI, send_server, strlen(send_server), 0);
    int reply_len = recv(sock_PI, receive, BUFFER, 0);
    receive[reply_len] = '\0';
    if (strstr(receive, "200")) {
        //success, get file name
        char send_server[BUFFER] = "file name";
        send(sock_PI, send_server, strlen(send_server), 0);
        reply_len = recv(sock_PI, receive, BUFFER, 0);
        receive[reply_len] = '\0';
        printf("%s\n", receive);
        // return file name
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

void check_output(char *cwd) {
    char absolute_output[BUFFER];
    sprintf(absolute_output, "%s/%s", cwd, OUTPUT);
    mkdir(absolute_output, S_IRWXU);
}

bool can_write(char* file_name, char* decrypt_path) {
    FILE* f;
    if((f = fopen(decrypt_path, "r"))) {
        fclose(f);
        char user_input[BUFFER];
        printf("File %s already exists.\nWould you like to overwrite it? [y/n]\n", file_name);
        fgets(user_input, BUFFER, stdin);
        if(strstr(user_input, "y") || strstr(user_input, "Y")) {
            remove(decrypt_path);
            return true;
        } else {
            // file is received but not written
            return false;
        }
    }
    return true;
}

void file_recv(char* file_name, char *cwd) {
    char decrypt_path[BUFFER];
    sprintf(decrypt_path, "%s/%s/%s", cwd, OUTPUT, file_name);
    if(can_write(file_name, decrypt_path)) {
        printf("%s\n", "Receiving data ...");
        // write to file using a byte array;
        // data port ready to receive bytes
        int file_len = get_file_len();
        char* file_bytes = malloc(file_len);
        // receive the file from the server
        recv(sock_DTP, file_bytes, file_len, 0);
        //confirm file received
        char confirm_end[BUFFER] = "[200] file received";
        send(sock_DTP, confirm_end, strlen(confirm_end), 0);
        printf("%s\n", confirm_end);
        printf("%s\n", "Processing file ...");
        char absolute_path[BUFFER];
        sprintf(absolute_path, "%s/%s/%s%s%s", cwd, OUTPUT, ENCRYPTED_TAG, file_name, ENCRYPTED_EXT);
        check_output(cwd);
        FILE* out = fopen(absolute_path, "w");
        fwrite(file_bytes, 1, file_len, out);
        fclose(out);
        // decryption process
        printf("%s\n", "Decrypting file ...");
        if(JNI_encrypt(decrypt_path, pass, "decrypt", cwd)) {
            printf("%s\n", "Decryption successful.");
            // Delete encrypted file
            remove(absolute_path);
        } else {
            printf("%s\n", "Decryption failed.");
            check_log(cwd);
        }
        free(file_bytes);
    } else {
        printf("%s\n", "Process terminated.");
    }

}

bool dispatch(char* input, char *cwd) {
    if(strstr(input, "ECHO")) {
        echo_loop();
    } else if(strstr(input, "LIST")) {
        serial_recv();
    } else if(strstr(input, "HELP")) {
        serial_recv();
    } else if(strstr(input, "RETR")) {
        char* file_name = check_input();
        if(file_name) {
            file_recv(file_name, cwd);
        } else {
            printf("%s\n", "args not ok.");
        }
        // freeing memory of path variable
        free(file_name);
    }else if(strstr(input, "QUIT")) {
        return false;
    }
    return true;
}