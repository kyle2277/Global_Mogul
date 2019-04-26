#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "jni_encryption.h"

#define FONT_BLANC "FontBlancMain"
#define FONT_BLANC_PATH "../JNI/Font_Blanc/java"
#define FONT_BLANC_LOG "../JNI/Font_Blanc/log.txt"
#define EJML_SIMPLE "../JNI/Font_Blanc/bin/ejml-simple-0.38.jar"
#define EJML_CORE "../JNI/Font_Blanc/bin/ejml-core-0.38.jar"
#define EJML_DDENSE "../JNI/Font_Blanc/bin/ejml-ddense-0.38.jar"
#define COMMONS_LANG "../JNI/Font_Blanc/bin/commons-lang3-3.8.1.jar"

#ifdef _WIN32
#define PATH_SEPARATOR ';'
#else
#define PATH_SEPARATOR ':'
#endif

#define MAX_DATA 1024

JNIEnv *env;
JavaVM *jvm;
JavaVMInitArgs vm_args;
jclass cls;
jmethodID mid;

bool JNI_init(char *cwd) {
    if(FB_exists(cwd)) {
        JavaVMOption options[2];
        char classpath[MAX_DATA];
        sprintf(classpath, "-Djava.class.path=%s/%s%c%s/%s%c%s/%s%c%s/%s%c%s/%s", cwd, FONT_BLANC_PATH, PATH_SEPARATOR,
                cwd, EJML_SIMPLE, PATH_SEPARATOR, cwd, EJML_CORE, PATH_SEPARATOR, cwd, EJML_DDENSE, PATH_SEPARATOR,
                cwd, COMMONS_LANG);
        options[0].optionString = classpath;
        options[1].optionString = "-verbose:jni";
        memset(&vm_args, 0, sizeof(vm_args));
        vm_args.version = JNI_VERSION_1_8;
        vm_args.nOptions = 2;
        vm_args.options = options;
        vm_args.ignoreUnrecognized = JNI_FALSE;
        return (JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args) != JNI_ERR);
    }
}

int JNI_encrypt(char *full_path, char *encryptKey, char *encrypt, char *cwd) {
    cls = (*env)->FindClass(env, "FontBlancMain");
    jint num = 0;
    if(cls != 0) {
        mid = (*env)->GetStaticMethodID(env, cls, "main", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
        if(mid != 0) {
            const char *file_path_const = full_path;
            const char *encode_key_const = encryptKey;
            const char *encrypt_const = encrypt;
            const char *cwd_const = cwd;
            jstring file_path = (*env)->NewStringUTF(env, file_path_const);
            jstring encodeKey = (*env)->NewStringUTF(env, encode_key_const);
            jstring encrypt_command = (*env)->NewStringUTF(env, encrypt_const);
            jstring cwd_str = (*env)->NewStringUTF(env, cwd_const);
            num = (*env)->CallStaticIntMethod(env, cls, mid, file_path, encodeKey, encrypt_command, cwd_str);
            printf("%d\n", num);
            return (int) num;
        }
    }
}

bool FB_exists(char *cwd) {
    FILE* fb;
    char full_path[MAX_DATA];
    sprintf(full_path, "%s/%s/%s.class", cwd, FONT_BLANC_PATH, FONT_BLANC);
    if((fb = fopen(full_path, "r"))) {
        fclose(fb);
        return true;
    }
    printf("No encryption device found at '%s/%s/%s'\n", cwd, FONT_BLANC_PATH, FONT_BLANC);
    return false;
}

bool check_log(char *cwd) {
    char log_path[256];
    int log_path_len = strlen(cwd) + strlen(FONT_BLANC_LOG) + 1;
    snprintf(log_path, log_path_len, "%s/%s", cwd, FONT_BLANC_LOG);
    FILE* log;
    if((log = fopen(log_path, "r"))) {
        char ch;
        while((ch = fgetc(log)) != EOF) {
            printf("%c", ch);
        }
        printf("\n");
        fclose(log);
        return false;
    }
    return true;
}

void JNI_end() {
    printf("%s\n", "JVM terminated.");
    (*jvm)->DestroyJavaVM(jvm);
}