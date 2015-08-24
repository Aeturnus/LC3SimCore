cmake_minimum_required(VERSION 3.2)
project(LC3SimCore)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Werror")

set(SOURCE_FILES main.c)
add_executable(LC3SimCore ${SOURCE_FILES} main.c lc3.h lc3.c ldb.h ldb.c fileio.h fileio.c terminal.h terminal.c)