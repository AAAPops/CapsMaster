cmake_minimum_required(VERSION 3.13)
project(kbdtool C)

set(CMAKE_C_STANDARD 99)

add_executable(kbdtool main.c)

add_executable(tst-1 tst-1.c)