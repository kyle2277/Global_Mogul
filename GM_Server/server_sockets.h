//
// Created by kylej on 4/5/19.
//

#ifndef GM_SERVER_SERVER_SOCKETS_H
#define GM_SERVER_SERVER_SOCKETS_H
#define MAX_DATA 1024

int sock_PI; // reference to the server's Protocol Interpreter (PI) socket which talks to the client
int sock_DTP; // reference to the server's Data Transfer Process (DTP) socket which exchanges data with the client
int client_sock_PI; // reference to connected client PI socket
int client_sock_DTP; // reference to connected client DTP socket
int sockaddr_len;

void init_PI_socket();
void init_DTP_socket(int port);
void listen_PI();
void listen_DTP();
void connect_PI();
void connect_DTP();
void DTP_port(int port_num);

#endif //GM_SERVER_SERVER_SOCKETS_H
