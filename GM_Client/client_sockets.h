//
// Created by kylej on 4/5/19.
//

#ifndef GM_CLIENT_CLIENT_SOCKETS_H
#define GM_CLIENT_CLIENT_SOCKETS_H
#define BUFFER 1024

struct sockaddr_in remote_server_PI;
struct sockaddr_in remote_server_DTP;
int sock_PI; // socket descriptor for the client Process Interpreter (PI) socket
int sock_DTP; // socket descriptor for the client Data Transfer Process (STP) socket

void init_sockets(char *argv[]);
void connect_PI(int sockaddr_len);
void connect_DTP(int sockaddr_len);

#endif //GM_CLIENT_CLIENT_SOCKETS_H