cmake_minimum_required(VERSION 3.13)
project(kbdtool C)

set(CMAKE_C_STANDARD 99)

add_definitions(-DLOG_USE_COLOR)

add_executable(lockmaster main.c args.c log.c common.h)

#add_executable(tst-1 tst-1.c)