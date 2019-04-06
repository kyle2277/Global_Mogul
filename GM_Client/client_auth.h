//
// Created by kylej on 4/5/19.
//

#ifndef GM_CLIENT_CLIENT_AUTH_H
#define GM_CLIENT_CLIENT_AUTH_H
#define BUFFER 1024

char pass[BUFFER];
void clean_pass();
void set_pass(char input[]);
void send_auth();

#endif //GM_CLIENT_CLIENT_AUTH_H
