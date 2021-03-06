cmake_minimum_required(VERSION 3.7)
project(GM_Server C)

set(CMAKE_C_STANDARD 99)

find_package(JNI)

if(${JNI_FOUND})
    #use JNI vars
    message(STATUS "Found JNI: ${JNI_FOUND}")
    include_directories(${JNI_INCLUDE_DIRS})
    link_directories(${JNI_LIBRARIES})
    add_executable(GM_Server ${JAVA_INCLUDE_PATH} ${JAVA_INCLUDE_PATH2} gm_server.c server_auth.h server_auth.c server_sockets.c server_sockets.h core.c core.h ../JNI/jni_encryption.c ../JNI/jni_encryption.h)
    target_link_libraries(GM_Server ${JAVA_JVM_LIBRARY})
else()
    #use JAVA_HOME vars
    set(JAVA $ENV{JAVA_HOME})
    set(JNI_LIB ${JAVA}/jre/lib/amd64/server)
    set(JNI ${JAVA}/include)
    set(JNI_LINUX ${JNI}/linux)

    include_directories(${JNI} ${JNI_LINUX})
    link_directories(${JNI_LIB})

    add_executable(GM_Server ${JNI}/jni.h ${JNI_LINUX}/jni_md.h gm_server.c server_auth.h server_auth.c server_sockets.c server_sockets.h core.c core.h ../JNI/jni_encryption.c ../JNI/jni_encryption.h)
    target_link_libraries(GM_Server ${JNI_LIB}/libjvm.so)
endif()