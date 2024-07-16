cmake_minimum_required(VERSION 3.29.3)

project(DiskFormatter, VERSION 0.0.0.1
        DESCRIPTION "Seriously Shit Disk Formatter (dont use)"
        LANGUAGES C)

add_library(fat STATIC lib/fat.c lib/include/fat.h)
target_include_directories(fat PUBLIC lib/include)

add_executable(format src/main.c)
target_link_libraries(format PUBLIC fat)

