cmake_minimum_required(VERSION 3.12)
project(GM_Server C)

set(CMAKE_C_STANDARD 99)

set(JNI_LIB /usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/server)
set(JNI /usr/lib/jvm/java-8-openjdk-amd64/include)
set(JNI_LINUX /usr/lib/jvm/java-8-openjdk-amd64/include/linux)

include_directories(${JNI} ${JNI_LINUX})
link_directories(${JNI_LIB})

add_executable(GM_Server ${JNI}/jni.h ${JNI_LINUX}/jni_md.h gm_server.c server_auth.h server_auth.c server_sockets.c server_sockets.h core.c core.h ../JNI/jni_encryption.c ../JNI/jni_encryption.h)
target_link_libraries(GM_Server ${JNI_LIB}/libjvm.so)