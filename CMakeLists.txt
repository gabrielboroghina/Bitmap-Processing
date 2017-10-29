cmake_minimum_required(VERSION 3.8)
project(PC3)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        bmp_header.h
        image_processing.c
        )

add_executable(PC3 ${SOURCE_FILES})