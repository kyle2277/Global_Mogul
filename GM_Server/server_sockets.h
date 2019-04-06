//
// Created by kylej on 4/5/19.
//

#ifndef GM_SERVER_SERVER_SOCKETS_H
#define GM_SERVER_SERVER_SOCKETS_H
#define MAX_DATA 1024

void init_sockets(char *argv[], int sockaddr_len, struct sockaddr_in server_PI, struct sockaddr_in server_DTP);
void listen_PI();
void listen_DTP();
void connect_PI(int sockaddr_len, struct sockaddr_in client_PI);
void connect_DTP(int sockaddr_len, struct sockaddr_in client_DTP);

#endif //GM_SERVER_SERVER_SOCKETS_H
