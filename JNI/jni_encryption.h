//
// Created by kylej on 4/15/19.
//

#ifndef GM_SERVER_JNI_TEST_H
#define GM_SERVER_JNI_TEST_H

typedef enum { false, true } bool;
bool JNI_init();
int JNI_encrypt(char *file_path, char *encryptKey, char *encrypt);
bool FB_exists();
bool check_log();
void JNI_end();

#endif //GM_SERVER_JNI_TEST_H
