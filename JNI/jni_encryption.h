//
// Created by kylej on 4/18/19.
//

#ifndef GM_CLIENT_JNI_ENCRYPTION_H
#define GM_CLIENT_JNI_ENCRYPTION_H

typedef enum { false, true } bool;
bool JNI_init(char *cwd);
int JNI_encrypt(char *file_path, char *encryptKey, char *encrypt, char *cwd);
bool FB_exists(char *cwd);
bool check_log(char *cwd);
void JNI_end();

#endif //GM_CLIENT_JNI_ENCRYPTION_H
