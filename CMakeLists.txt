cmake_minimum_required(VERSION 3.6)
project(fpgaSorter)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c vdbesort_empty.c vdbesort_empty.h fpgaSorter.h fpgaSorter.c)
add_executable(fpgaSorter ${SOURCE_FILES})