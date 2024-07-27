add_library(fat fat.c)
add_library(blkDevices blkDevices.c)

target_include_directories(fat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(fat PRIVATE blkDevices)

