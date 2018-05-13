cmake_minimum_required(VERSION 3.9)
project(CSE240A)

set(CMAKE_CXX_STANDARD 11)

add_executable(CSE240A
        src/main.c
        src/predictor.c
        src/predictor.h
        traces/fp_1.bz2
        traces/fp_2.bz2
        traces/int_1.bz2
        traces/int_2.bz2
        traces/mm_1.bz2
        traces/mm_2.bz2
        README.md
        README.txt)
