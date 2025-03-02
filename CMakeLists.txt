cmake_minimum_required(VERSION 3.28)

project(Httpd VERSION 0.0.1 LANGUAGES CXX)

if (DEBUG_MODE)
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -g -Og)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -g -Og)
endif ()

set(MY_INCS "${CMAKE_CURRENT_SOURCE_DIR}/includes")
set(MY_LIBS "${CMAKE_CURRENT_SOURCE_DIR}/build")

add_subdirectory(src)
enable_testing()
add_subdirectory(tests)
