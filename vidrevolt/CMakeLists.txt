project (vidrevolt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} main.cpp)

target_compile_options(${PROJECT_NAME} PRIVATE "-Wextra" "-Werror" "-Wall" "-pedantic-errors")

include_directories(${CMAKE_SOURCE_DIR}/common)
target_link_libraries(${PROJECT_NAME} common)

# Libraries brought in by conan dependency manager
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})
