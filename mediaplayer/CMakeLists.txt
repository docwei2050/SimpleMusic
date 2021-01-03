
cmake_minimum_required(VERSION 3.4.1)

#导入头文件
include_directories(src/main/cpp/include)
add_library(
        native-lib
        SHARED
        src/main/cpp/native-lib.cpp
        src/main/cpp/CallJava.cpp
        src/main/cpp/FFmpegDecode.cpp
        src/main/cpp/SimpleAudio.cpp
        src/main/cpp/SimpleQueue.cpp
        src/main/cpp/PlayStatus.cpp
        src/main/cpp/SimpleVideo.cpp)

add_library(avcodec-58 SHARED IMPORTED)
set_target_properties(avcodec-58
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavcodec-58.so)


add_library(avdevice-58 SHARED IMPORTED)
set_target_properties(avdevice-58
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavdevice-58.so)


add_library(avfilter-7 SHARED IMPORTED)
set_target_properties(avfilter-7
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavfilter-7.so)


add_library(avformat-58 SHARED IMPORTED)
set_target_properties(avformat-58
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavformat-58.so)


add_library(avutil-56 SHARED IMPORTED)
set_target_properties(avutil-56
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavutil-56.so)

add_library(postproc-55 SHARED IMPORTED)
set_target_properties(postproc-55
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libpostproc-55.so)

add_library(swresample-3 SHARED IMPORTED)
set_target_properties(swresample-3
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libswresample-3.so)


add_library(swscale-5 SHARED IMPORTED)
set_target_properties(swscale-5
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libswscale-5.so)


target_link_libraries( # Specifies the target library.
        native-lib

        avcodec-58
        avdevice-58
        avfilter-7
        avformat-58
        avutil-56
        postproc-55
        swresample-3
        swscale-5

        OpenSLES
        # Links the target library to the log library
        # included in the NDK.
        log)

# Links the target library to the log library
# included in the NDK.
