
cmake_minimum_required(VERSION 3.4.1)

#导入头文件
include_directories(src/main/cpp/includes)
#添加动态库
add_library(avcodec SHARED IMPORTED)
#设置动态库的路径
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}/libavcodec.so)


#添加动态库
add_library(avdevice SHARED IMPORTED)
#设置动态库的路径
set_target_properties(avdevice PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavdevice.so)

#添加动态库
add_library(avfilter SHARED IMPORTED)
#设置动态库的路径
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavfilter.so)

#添加动态库
add_library(avformat SHARED IMPORTED)
#设置动态库的路径
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavformat.so)

#添加动态库
add_library(avresample SHARED IMPORTED)
#设置动态库的路径
set_target_properties(avresample PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavresample.so)

#添加动态库
add_library(avutil SHARED IMPORTED)
#设置动态库的路径
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavutil.so)

#添加动态库
add_library(swresample SHARED IMPORTED)
#设置动态库的路径
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libswresample.so)

#添加动态库
add_library(swscale SHARED IMPORTED)
#设置动态库的路径
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libswscale.so)


add_library(
        native-lib
        SHARED
        src/main/cpp/native-lib.cpp)


find_library(
        log-lib
        log)
SET(USER_KEY, "Hello World")
MESSAGE(STATUS "this var key = ${USER_KEY}.")

target_link_libraries(
        native-lib
        ${log-lib}
        avcodec
        avdevice
        avfilter
        avformat
        avresample
        avutil
        swresample
        swscale)