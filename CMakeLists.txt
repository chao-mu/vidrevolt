cmake_minimum_required (VERSION 3.10)
project (vidrevolt-global)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wextra -Werror -Wall -pedantic-errors -Wconversion")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_compile_options("-Wextra" "-Werror" "-Wall" "-pedantic-errors" "-Wconversion")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(common)
add_subdirectory(vidrevolt)
#add_subdirectory(exp)
#add_subdirectory(textexp)
#add_subdirectory(vidrevolt-pre)
#add_subdirectory(videxp)
#add_subdirectory(videxp2)
