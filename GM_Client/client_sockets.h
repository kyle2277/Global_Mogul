//
// Created by kylej on 4/5/19.
//

#ifndef GM_CLIENT_CLIENT_SOCKETS_H
#define GM_CLIENT_CLIENT_SOCKETS_H
#define BUFFER 1024
#include <WinSock2.h>

#ifdef _WIN32
SOCKET sock_PI;
SOCKET sock_DTP;
#else
int sock_PI; // socket descriptor for the client Process Interpreter (PI) socket
int sock_DTP; // socket descriptor for the client Data Transfer Process (STP) socket
#endif
struct sockaddr_in remote_server_PI;
struct sockaddr_in remote_server_DTP;
char server_addr[256];
int sockaddr_len;

#ifdef _WIN32
void init_Winsock();
#endif
void resetErrno();
void init_PI_socket();
void init_DTP_socket(int port);
void connect_PI();
void connect_DTP();
void DTP_port(int port_num);
void set_server_addr(char *address);
void shutdownAll();

#endif //GM_CLIENT_CLIENT_SOCKETS_H