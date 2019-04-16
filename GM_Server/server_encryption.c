#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "core.h"
#include "server_auth.h"
#include "server_encryption.h"

#define FONT_BLANC_PATH "/home/kylej/Documents/Kyle/Dev/ProjectFB/Font_Blanc"
#define FONT_BLANC "FontBlancMain"
#define FONT_BLANC_LOG "/home/kylej/Documents/Kyle/Dev/ProjectFB/Font_Blanc/log.txt"

/*
 * ALL DEPRECATED
 */

bool encrypt(char* file_path) {
    if(FB_exists()) {
        char* command = malloc(MAX_DATA);
        sprintf(command, "cd %s && java %s %s %s encrypt", FONT_BLANC_PATH, FONT_BLANC, file_path, pass);
        int sys = system("cd /home/kylej/Documents/Kyle/Dev/ProjectFB/Font_Blanc && java FontBlancMain ./user/vtec.txt password encrypt");
        free(command);
        return check_log();
    }
    return false;
}

bool FB_exists() {
    FILE* fb;
    char full_path[MAX_DATA];
    sprintf(full_path, "%s/%s.class", FONT_BLANC_PATH, FONT_BLANC);
    if((fb = fopen(full_path, "r"))) {
        fclose(fb);
        return true;
    }
    printf("No encryption device found at '%s/%s'\n", FONT_BLANC_PATH, FONT_BLANC);
    return false;
}

bool check_log() {
    FILE* log;
    char* log_path = malloc(MAX_DATA);
    sprintf(log_path, "%s", FONT_BLANC_LOG);
    if((log = fopen(log_path, "r"))) {
        char ch;
        while((ch = fgetc(log)) != EOF) {
            printf("%c", ch);
        }
        printf("\n");
        fclose(log);
        return false;
    }
    free(log_path);
    return true;
}

