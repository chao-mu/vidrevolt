project (vidrevolt)

add_executable(${PROJECT_NAME} main.cpp)

include_directories(${CMAKE_SOURCE_DIR}/common)
target_link_libraries(${PROJECT_NAME} common)
