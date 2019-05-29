#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../JNI/jni_encryption.h"
#include "client_sockets.h"
#include "client_auth.h"

#define OUTPUT "output"
#define ENCRYPTED_TAG "encrypted_"
#define ENCRYPTED_EXT ".txt"

#ifdef _WIN32
//Windows system
#include <WinSock2.h>
#include <direct.h>
#define ERROR SOCKET_ERROR
#define GET_ERROR WSAGetLastError()

#else

//Linux and Mac OSX systems
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#define ERROR -1
#define GET_ERROR errno

#endif

void sendData(int socket, char *sendStr, int strLen, int flags) {
    resetErrno();
    int status = send(socket, sendStr, strLen, flags);
    if(status == ERROR) {
        printf("Failed to send data with error: %d\n", GET_ERROR);
    }
}

int recvData(int socket, char *recvStr, int recvBuffer, int flags) {
    resetErrno();
    int status = recv(socket, recvStr, recvBuffer, flags);
    if(status == ERROR) {
        printf("Failed to receive data with error: %d\n", GET_ERROR);
    }
    return status;
}

/* DEPRECATED */
void echo_loop() {
    int reply_len;
    char input[BUFFER]; // stores user input
    char output[BUFFER]; // stores remote_server response
    bool run = true;
    while(run) {
        // fgets() reads input (containing spaces) from user, stores in provided string (input)
        fgets(input, BUFFER, stdin);
        char *quit = strstr(input, "QUIT");
        sendData(sock_PI, input, strlen(input), 0);
        // check if the user wants to terminate the program
        if(quit) {
            run = false;
        }
        reply_len = recvData(sock_DTP, output, BUFFER, 0);
        output[reply_len] = '\0';
        printf("%s\n", output);
    }
}

void serial_recv() {
    int reply_len;
    char receive[BUFFER]; // stores server response
    char *received = "200"; // confirmation to server when data packet received
    // lets server know data port is ready to receive data
    char *data_ready = "DTP ready";
    sendData(sock_DTP, data_ready, strlen(data_ready), 0);
    reply_len = recvData(sock_DTP, receive, BUFFER, 0);
    do {
        receive[reply_len] = '\0';
        printf("%s\n", receive);
        sendData(sock_DTP, received, strlen(received), 0);
        reply_len = recvData(sock_DTP, receive, BUFFER, 0);
    } while (!strstr(receive, "end"));
    char *confirm_end = "end received";
    sendData(sock_DTP, confirm_end, strlen(confirm_end), 0);
}

bool can_write(char* file_name, char* decrypt_path) {
    FILE* f;
    if((f = fopen(decrypt_path, "r"))) {
        fclose(f);
        char user_input[4];
        printf("File %s already exists.\nWould you like to overwrite it? [y/n]\n", file_name);
        fgets(user_input, 4, stdin);
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

long get_file_len() {
    char length_str[BUFFER];
    //receive file length
    int reply_len = recvData(sock_PI, length_str, BUFFER, 0);
    length_str[reply_len] = '\0';
    long length = strtol(length_str, NULL, 10);
    printf("file length: %ld\n", length);
    return length;
}

void check_output(char *cwd) {
    char absolute_output[BUFFER];
    sprintf(absolute_output, "%s/%s", cwd, OUTPUT);
#ifdef _WIN32
	_mkdir(absolute_output);
#else
	mkdir(absolute_output, S_IRWXU);
#endif
	
}

void receive_packets(char *file_bytes, long num_packets, long packet_size, long last_packet, FILE* out) {
	char *packet_bytes = malloc(packet_size);
    for(int i = 0; i < num_packets; i++) {
        recvData(sock_DTP, packet_bytes, packet_size, 0);
//        snprintf(file_bytes, strlen(file_bytes) + packet_size, "%s%s", file_bytes, packet_bytes);
        fwrite(packet_bytes, 1, packet_size, out);
        sendData(sock_DTP, "200 OK", strlen("200 OK"), 0);
    }
    free(packet_bytes);
    //last packet
    char *last_pkt = malloc(last_packet);
    recvData(sock_DTP, packet_bytes, last_packet, 0);
    snprintf(file_bytes, strlen(file_bytes) + last_packet, "%s%s", file_bytes, packet_bytes);
    fwrite(packet_bytes, 1, last_packet, out);
    free(last_pkt);
    sendData(sock_DTP, "200 OK", strlen("200 OK"), 0);
}

void file_recv(char* file_name, char *decrypt_path, char *cwd) {
    int len_received;
    char *file_bytes;

    printf("%s\n", "Receiving data ...");
    // write to file using a byte array;
    // data port ready to receive bytes
    char absolute_path[BUFFER];
    sprintf(absolute_path, "%s/%s/%s%s%s", cwd, OUTPUT, ENCRYPTED_TAG, file_name, ENCRYPTED_EXT);
    check_output(cwd);
    FILE* out = fopen(absolute_path, "w");
    long file_len = get_file_len();
    file_bytes = malloc(file_len);
    if(file_len <= BUFFER) {
        // receive the file from the server
        len_received = recvData(sock_DTP, file_bytes, file_len, 0);
        printf("bytes received: %d\n", len_received);
        fwrite(file_bytes, 1, file_len, out);
    } else {
        //prepare to receive packets
        //get packet size
        char packet_size_str[256];
        long packet_size;
        sendData(sock_PI, "packet size?", strlen("packet size?"), 0);
        len_received = recvData(sock_PI, packet_size_str, BUFFER, 0);
        packet_size_str[len_received] = '\0';
        printf("Packet size: %s\n", packet_size_str);
        packet_size = strtol(packet_size_str, NULL, 10);
        //get last packet size
        sendData(sock_PI, "last packet size?", strlen("last packet size?"), 0);
        char last_packet_str[256];
        len_received = recvData(sock_PI, last_packet_str, BUFFER, 0);
        last_packet_str[len_received] = '\0';
        printf("size of last packet: %s\n", last_packet_str);
        long last_packet = strtol(last_packet_str, NULL, 10);
        // get number of packets
        sendData(sock_PI, "num packets?", strlen("num packets?"), 0);
        char num_packets_str[256];
        len_received = recvData(sock_PI, num_packets_str, BUFFER, 0);
        num_packets_str[len_received] = '\0';
        printf("Number packets: %s\n", num_packets_str);
        long num_packets = strtol(num_packets_str, NULL, 10);
        //receive packets
        receive_packets(file_bytes, num_packets, packet_size, last_packet, out);
    }

    printf("%s\n", "Processing file ...");
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
    //confirm file received
    char *confirm_end = "[200] file received";
    sendData(sock_DTP, confirm_end, strlen(confirm_end), 0);
    printf("%s\n", confirm_end);

}

void check_input(char *cwd) {
    char send_server[256] = "file name?";
    char *file_name = malloc(BUFFER);
    sendData(sock_PI, send_server, strlen(send_server), 0);
    int reply_len = recvData(sock_PI, file_name, BUFFER, 0);
    file_name[reply_len] = '\0';
    if(!strstr(file_name, "ERROR")) {
        printf("%s\n", file_name);
        char decrypt_path[BUFFER];
        sprintf(decrypt_path, "%s/%s/%s", cwd, OUTPUT, file_name);
        if(can_write(file_name, decrypt_path)) {
            char *can_send = "200 can send";
            sendData(sock_PI, can_send, strlen(can_send), 0);
            file_recv(file_name, decrypt_path, cwd);
        } else {
            // do not send file
            printf("%s\n", "Process terminated.");
        }
    } else {
        printf("%s\n", file_name);
        //failure
    }

}

void test_DTP_connection() {
    char *send_server = "200";
    sendData(sock_DTP, send_server, strlen(send_server), 0);
    char receive[BUFFER];
    int len = recvData(sock_DTP, receive, BUFFER, 0);
    receive[len] = '\0';
    printf("%s\n", receive);
}

void port() {
    char send_server[256] = "delete DTP?";
    send_server[strlen(send_server)] = '\0';
    sendData(sock_PI, send_server, strlen(send_server), 0);
    char receive[BUFFER];
    int len = recvData(sock_PI, receive, BUFFER, 0);
    receive[len] = '\0';
    printf("%s\n", receive);
    //recieve 200 or error message
    if(strstr(receive, "200")) {
        //delete DTP port
        //ask port number
        strcpy(send_server, "Port No.?");
        sendData(sock_PI, send_server, strlen(send_server), 0);
        printf("%s\n", send_server);
        len = recvData(sock_PI, receive, BUFFER, 0);
        receive[len] = '\0';
        printf("%s\n", receive);
        DTP_port(atoi(receive));
        test_DTP_connection();
    }
}

bool dispatch(char* input, char *cwd) {
    if(strstr(input, "ECHO")) {
        echo_loop();
    } else if(strstr(input, "LIST")) {
        serial_recv();
    } else if(strstr(input, "HELP")) {
        serial_recv();
    } else if(strstr(input, "PORT")) {
        port();
    } else if(strstr(input, "RETR")) {
        check_input(cwd);
    }else if(strstr(input, "QUIT")) {
        return false;
    }
    return true;
}