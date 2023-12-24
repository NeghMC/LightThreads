cmake_minimum_required(VERSION 3.5)
project(LightThreads)
set(LIB_NAME lightThreads)
add_library(
    ${LIB_NAME}
    lightThreads.c
    lightThreads.h
)
target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})