//
// Created by kylej on 4/5/19.
//

#ifndef GM_CLIENT_CLIENT_SOCKETS_H
#define GM_CLIENT_CLIENT_SOCKETS_H
#define BUFFER 1024

int sock_PI; // socket descriptor for the client Process Interpreter (PI) socket
int sock_DTP; // socket descriptor for the client Data Transfer Process (STP) socket
char server_addr[256];

void init_PI_socket(struct sockaddr_in remote_server_PI);
void init_DTP_socket(struct sockaddr_in remote_server_DTP, char *port);
void connect_PI(int sockaddr_len, struct sockaddr_in remote_server_PI);
void connect_DTP(int sockaddr_len, struct sockaddr_in remote_server_DTP);
void DTP_port(char *port_num);
void set_server_addr(char *address);

#endif //GM_CLIENT_CLIENT_SOCKETS_H