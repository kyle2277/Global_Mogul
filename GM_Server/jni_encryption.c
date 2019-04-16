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

bool JNI_encrypt(char *full_path) {
    if(FB_exists()) {
        JNIEnv *env;
        JavaVM *jvm;
        JavaVMInitArgs vm_args;
        long status;
        jclass cls;
        jmethodID mid;
        JavaVMOption options[1];
        char *encrypt = "encrypt";

        int classpath_len = strlen("-Djava.class.path=") + strlen(FONT_BLANC_PATH) + strlen(CLASSPATH);
        char classpath[classpath_len];
        snprintf(classpath, classpath_len, "-Djava.class.path=%s:%s", FONT_BLANC_PATH, CLASSPATH);
        options[0].optionString = classpath;
        memset(&vm_args, 0, sizeof(vm_args));
        vm_args.version = JNI_VERSION_1_8;
        vm_args.nOptions = 1;
        vm_args.options = options;
        status = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);

        if(status != JNI_ERR) {
            cls = (*env)->FindClass(env, "FontBlancMain");
            if(cls != 0) {
                mid = (*env)->GetStaticMethodID(env, cls, "main", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
                if(mid != 0) {
                    jstring file_path = (*env)->NewStringUTF(env, full_path);
                    jstring encodeKey = (*env)->NewStringUTF(env, "password");
                    jstring encrypt_command = (*env)->NewStringUTF(env, encrypt);
                    jint num = (*env)->CallStaticIntMethod(env, cls, mid, file_path, encodeKey, encrypt_command);
                    printf("%d\n", num);
                    /*jstring result = (*env)->CallStaticObjectMethod(env, cls, mid);
                    const char* str = (*env)->GetStringUTFChars(env, result, 0);
                    printf("%s\n", str);
                    (*env)->ReleaseStringUTFChars(env, result, str);
                    */
                 }
            }
            (*jvm)->DestroyJavaVM(jvm);
            return true;
        } else {
            return false;
        }
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
        free(log_path);
        return false;
    }
    free(log_path);
    return true;
}



