cmake_minimum_required(VERSION 3.29.3)

project(DiskFormatter, VERSION 0.0.0.1
        DESCRIPTION "Seriously Shit Disk Formatter (dont use)"
        LANGUAGES C)

add_subdirectory(./lib)

add_executable(format ./src/main.c)

target_link_libraries(format PUBLIC fat)

target_include_directories(format PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           "${PROJECT_SOURCE_DIR}/lib/include"
                       )

