cmake_minimum_required(VERSION 3.7)
project(GM_Client C)

set(CMAKE_C_STANDARD 99)

set(JAVA $ENV{JAVA_HOME})

if(WIN32)
    #path separator
    set(SEP \\)
    set(JNI ${JAVA}${SEP}include)
    #set windows JNI variables
    set(JNI_LIB ${JAVA}${SEP}jre${SEP}bin${SEP}server${SEP}jvm.dll)
    set(JNI_OS ${JNI}${SEP}win32)
elseif(APPLE)
    #path separator
    set(SEP /)
    set(JNI ${JAVA}${SEP}include)
    #set mac JNI variables
    set(JNI_LIB ${JAVA}${SEP}jre${SEP}lib${SEP}server${SEP}libjvm.dylib)
    set(JNI_OS ${JNI}${SEP}darwin)
elseif(UNIX AND NOT APPLE)
    #path separator
    set(SEP /)
    set(JNI ${JAVA}${SEP}include)
    #set linux JNI variables
    set(JNI_LIB ${JAVA}${SEP}jre${SEP}lib${SEP}amd64${SEP}server${SEP}libjvm.so)
    set(JNI_OS ${JNI}${SEP}linux)
endif()

include_directories(${JNI} ${JNI_OS})
link_directories(${JNI_LIB})

add_executable(GM_Client ${JNI}${SEP}jni.h ${JNI_OS}${SEP}jni_md.h gm_client.c client_sockets.c client_sockets.h client_auth.c client_auth.h core.c core.h ..${SEP}JNI${SEP}jni_encryption.c ..${SEP}JNI${SEP}jni_encryption.h)
target_link_libraries(GM_Client ${JNI_LIB})
