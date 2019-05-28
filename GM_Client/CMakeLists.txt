cmake_minimum_required(VERSION 3.7)
project(GM_Client C)

set(CMAKE_C_STANDARD 99)

set(JAVA $ENV{JAVA_HOME})
string(REPLACE "\\" "/" JAVA ${JAVA})

if(WIN32)
    #path separator
    set(SEP \\)
    set(JNI ${JAVA}/include)
    string(REPLACE ${SEP} "/" JNI ${JNI})
    #set windows JNI variables
    set(JNI_LIB ${JAVA}/jre/bin/server/jvm.dll)
    string(REPLACE ${SEP} "/" JNI_LIB ${JNI_LIB})
    set(JNI_OS ${JNI}/win32)
    string(REPLACE ${SEP} "/" JNI_OS ${JNI_OS})
elseif(APPLE)
    set(JNI ${JAVA}/include)
    #set mac JNI variables
    set(JNI_LIB ${JAVA}/jre/lib/server/libjvm.dylib)
    set(JNI_OS ${JNI}/darwin)
elseif(UNIX AND NOT APPLE)
    set(JNI ${JAVA}/include)
    #set linux JNI variables
    set(JNI_LIB ${JAVA}/jre/lib/amd64/server/libjvm.so)
    set(JNI_OS ${JNI}/linux)
endif()

include_directories(${JNI} ${JNI_OS})
link_directories(${JNI_LIB})

add_executable(GM_Client ${JNI}/jni.h ${JNI_OS}/jni_md.h gm_client.c client_sockets.c client_sockets.h client_auth.c client_auth.h core.c core.h ../JNI/jni_encryption.c ../JNI/jni_encryption.h)
target_link_libraries(GM_Client ${JNI_LIB})
