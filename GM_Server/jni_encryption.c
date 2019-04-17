#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server_auth.h"
#include "jni_encryption.h"

#define FONT_BLANC_PATH "/home/kylej/Documents/Kyle/Dev/ProjectFB/Font_Blanc"
#define CLASSPATH "/home/kylej/Documents/Kyle/Dev/ProjectFB/Font_Blanc:/home/kylej/Documents/Kyle/Dev/Arkadiusz2.0/ejml/ejml-simple-0.37.1.jar:/home/kylej/Documents/Kyle/Dev/Arkadiusz2.0/ejml/ejml-core-0.37.1.jar:/home/kylej/Documents/Kyle/Dev/Arkadiusz2.0/ejml/ejml-ddense-0.37.1.jar:/home/kylej/Documents/Kyle/Dev/Arkadiusz2.0/commons-lang3-3.8.1/commons-lang3-3.8.1.jar:."
#define FONT_BLANC "FontBlancMain"
#define FONT_BLANC_LOG "/home/kylej/Documents/Kyle/Dev/ProjectFB/Font_Blanc/log.txt"

#ifdef _WIN32
#define PATH_SEPARATOR ';'
#else
#define PATH_SEPARATOR ':'
#endif

int JNI_encrypt(char *full_path) {
    if(FB_exists()) {
        JNIEnv *env;
        JavaVM *jvm;
        JavaVMInitArgs vm_args;
        long status;
        jclass cls;
        int result = 0;
        jmethodID mid;
        JavaVMOption options[2];
        char *encrypt = "encrypt";

        int classpath_len = strlen("-Djava.class.path=") + strlen(FONT_BLANC_PATH) + strlen(CLASSPATH);
        char classpath[classpath_len];
        snprintf(classpath, classpath_len, "-Djava.class.path=%s:%s", FONT_BLANC_PATH, CLASSPATH);
        options[0].optionString = classpath;
        options[1].optionString = "-verbose:jni";
        memset(&vm_args, 0, sizeof(vm_args));
        vm_args.version = JNI_VERSION_1_8;
        vm_args.nOptions = 2;
        vm_args.options = options;
        vm_args.ignoreUnrecognized = JNI_FALSE;
        status = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);

        if(status != JNI_ERR) {
            cls = (*env)->FindClass(env, "FontBlancMain");
            jint num = 0;
            if(cls != 0) {
                mid = (*env)->GetStaticMethodID(env, cls, "main", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
                if(mid != 0) {
                    const char *file_path_const = full_path;
                    const char *encode_key_const = pass;
                    const char *encrypt_const = encrypt;
                    jstring file_path = (*env)->NewStringUTF(env, file_path_const);
                    jstring encodeKey = (*env)->NewStringUTF(env, encode_key_const);
                    jstring encrypt_command = (*env)->NewStringUTF(env, encrypt_const);
                    num = (*env)->CallStaticIntMethod(env, cls, mid, file_path, encodeKey, encrypt_command);
//                    (*env)->ReleaseStringUTFChars(env, file_path, full_path);
//                    (*env)->ReleaseStringUTFChars(env, encodeKey, pass);
//                    (*env)->ReleaseStringUTFChars(env, encrypt_command, encrypt);
                    (*jvm)->DestroyJavaVM(jvm);
                    printf("%d\n", num);

                }

                return (int) num;
            }
        }
    }
    return 0;
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
        free(log_path);
        return false;
    }
    free(log_path);
    return true;
}



