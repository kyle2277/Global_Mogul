cmake_minimum_required(VERSION 3.7)
project(GM_Client C)

set(CMAKE_C_STANDARD 99)

set(JAVA $ENV{JAVA_HOME})
set(JNI ${JAVA}/include)

if(WIN32)
    #set windows JNI variables
elseif(APPLE)
    #set mac JNI variables
    set(JNI_LIB ${JAVA}/jre/lib/server/libjvm.dylib)
    set(JNI_OS ${JNI}/darwin)
elseif(UNIX AND NOT APPLE)
    #set linux JNI variables
    set(JNI_LIB ${JAVA}/jre/lib/amd64/server/libjvm.so)
    set(JNI_OS ${JNI}/linux)
endif()

include_directories(${JNI} ${JNI_OS})
link_directories(${JNI_LIB})

add_executable(GM_Client ${JNI}/jni.h ${JNI_OS}/jni_md.h gm_client.c client_sockets.c client_sockets.h client_auth.c client_auth.h core.c core.h ../JNI/jni_encryption.c ../JNI/jni_encryption.h)
target_link_libraries(GM_Client ${JNI_LIB})
