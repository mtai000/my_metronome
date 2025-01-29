cmake_minimum_required(VERSION 3.22.1)

project("engine")

set(OBOE_DIR ${PROJECT_SOURCE_DIR}/../../../../oboe)
set(DEBUG_UTILS_PATH ${OBOE_DIR}/samples/debug-utils)

add_subdirectory(${OBOE_DIR} ${CMAKE_BINARY_DIR}/oboe)
add_library(${CMAKE_PROJECT_NAME} SHARED
        EngineNative.cpp
        Demo.cpp
        Demo.h
        MPM.h
        MPM.cpp
        ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.h
        ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c
        ${DEBUG_UTILS_PATH}/trace.cpp
        )
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${OBOE_DIR}/include
        ${OBOE_DIR}/samples/shared
        ${ANDROID_NDK}/sources/android/cpufeatures
        ${DEBUG_UTILS_PATH}
)

target_link_libraries(${CMAKE_PROJECT_NAME}
        oboe
        android
        log)