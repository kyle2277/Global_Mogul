#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server_auth.h"

#define FONT_BLANC_PATH "/home/kylej/Documents/Kyle/Dev/ProjectFB/Font_Blanc"
#define FONT_BLANC "FontBlancMain"
#define FONT_BLANC_LOG "/home/kylej/Documents/Kyle/Dev/ProjectFB/Font_Blanc/log.txt"

#ifdef _WIN32
#define PATH_SEPARATOR ';'
#else
#define PATH_SEPARATOR ':'
#endif

int JNI_encrypt(char *file_path) {
    JNIEnv *env;
    JavaVM *jvm;
    JavaVMInitArgs vm_args;
    long status;
    jclass cls;
    jmethodID mid;
    jint num;
    JavaVMOption options[1];

    int classpath_len = strlen("-Djava.class.path=") + strlen(FONT_BLANC_PATH);
    char classpath[classpath_len];
    snprintf(classpath, classpath_len, "-Djava.class.path=%s", FONT_BLANC_PATH);
    options[0].optionString = classpath;
    memset(&vm_args, 0, sizeof(vm_args));
    vm_args.version = JNI_VERSION_1_8;
    vm_args.nOptions = 1;
    vm_args.options = options;
    status = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);

    if(status != JNI_ERR) {
        cls = (*env)->FindClass(env, "FontBlancMain");
        if(cls != 0) {
            mid = (*env)->GetStaticMethodID(env, cls, "main", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
            if(mid != 0) {
                num = (*env)->CallStaticIntMethod(env, cls, mid, file_path, pass, "encrypt");
                printf("%d\n", num);
            }
        }
        (*jvm)->DestroyJavaVM(jvm);
        return 0;
    } else {
        return -1;
    }

}



