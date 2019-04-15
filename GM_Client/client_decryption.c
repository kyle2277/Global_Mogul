#include <stdio.h>
#include <stdlib.h>

#include "client_decryption.h"
#include "client_auth.h"

#define FONT_BLANC "/home/kylej/Documents/Kyle/Dev/ProjectFB/Font_Blanc/FontBlancMain"
#define FONT_BLANC_LOG "/home/kylej/Documents/Kyle/Dev/ProjectFB/Font_Blanc/log.txt"
#define ENCRYPTED_TAG "encrypted_"

// change use of sprintf

bool decrypt(char* full_path) {
    if(FB_exists()) {
        char* command = malloc(BUFFER);
        sprintf(command, "java %s %s %s decrypt", FONT_BLANC, full_path, pass);
        free(command);
        return check_log();
    }
    return false;
}

bool FB_exists() {
    FILE* fb;
    if((fb = fopen(FONT_BLANC, "r"))) {
        fclose(fb);
        return true;
    }
    printf("No decryption device found at '%s'\n", FONT_BLANC);
    return false;
}

bool check_log() {
    FILE* log;
    if((log = fopen(FONT_BLANC_LOG, "r"))) {
        char ch;
        while((ch = fgetc(log)) != EOF) {
            printf("%c", ch);
        }
        fclose(log);
        return false;
    }
    return true;
}
