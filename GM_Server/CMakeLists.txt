cmake_minimum_required(VERSION 3.12)
project(GM_Server C)

set(CMAKE_C_STANDARD 99)

set(JAVA $ENV{JAVA_HOME})

set(JNI_LIB ${JAVA}/jre/lib/amd64/server)
set(JNI ${JAVA}/include)
set(JNI_LINUX ${JNI}/linux)

include_directories(${JNI} ${JNI_LINUX})
link_directories(${JNI_LIB})

add_executable(GM_Server ${JNI}/jni.h ${JNI_LINUX}/jni_md.h gm_server.c server_auth.h server_auth.c server_sockets.c server_sockets.h core.c core.h ../JNI/jni_encryption.c ../JNI/jni_encryption.h)
target_link_libraries(GM_Server ${JNI_LIB}/libjvm.so)