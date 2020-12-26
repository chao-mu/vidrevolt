cmake_minimum_required (VERSION 3.10)
project (vidrevolt)

option(COVERAGE "Enable code coverage reporting" FALSE)

if (COVERAGE)
    # Disable code optimizations
    add_compile_options(--coverage -O0)
    MESSAGE(STATUS "Coverage enabled!")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenGL_GL_PREFERENCE GLVND)

#
# Conan (Dependency Management) fun!
#
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_check(VERSION 1.0.0 REQUIRED)

conan_add_remote(
    NAME vidrevolt
    URL https://api.bintray.com/conan/vidrevolt/conan-packages
)

conan_add_remote(
    NAME bincrafters
    URL https://api.bintray.com/conan/bincrafters/public-conan
)

conan_add_remote(
    NAME martin
    URL https://api.bintray.com/conan/martinmoene/nonstd-lite
)

conan_add_remote(
    NAME flexferrum
    URL https://api.bintray.com/conan/flexferrum/conan-packages
)

conan_add_remote(
    NAME vthiery
    URL https://api.bintray.com/conan/vthiery/conan-packages
)

conan_add_remote(
    NAME DEGoodmanWilson
    URL https://api.bintray.com/conan/degoodmanwilson/opensource
)

#set (CONAN_SETTINGS SETTINGS compiler.libcxx=libc++)
conan_cmake_run(
    REQUIRES
        inja/[~=1.0]@DEGoodmanWilson/stable
        bzip2/1.0.8@conan/stable
        yaml-cpp/0.6.2@bincrafters/stable
        tclap/1.2.2@vidrevolt/stable
        #gtest/1.8.1@bincrafters/stable
        glad/0.1.29@bincrafters/stable
    BASIC_SETUP
    #    ${CONAN_SETTINGS}
    OPTIONS
        #sfml:audio=True
        glad:api_version=4.1
        glad:spec=gl
    BUILD missing
)

#
# Main executable
#
add_executable(${PROJECT_NAME} src/main.cpp src/Keyboard.cpp src/BPMSync.cpp src/AddressOrValue.cpp src/Video.cpp src/midi/Device.cpp src/midi/Message.cpp src/midi/Control.cpp src/Image.cpp src/osc/Server.cpp src/Pipeline.cpp src/Value.cpp src/Address.cpp src/gl/Texture.cpp src/gl/GLUtil.cpp src/gl/ShaderProgram.cpp src/gl/RenderOut.cpp src/gl/IndexBuffer.cpp src/gl/Renderer.cpp src/gl/VertexArray.cpp src/gl/VertexBuffer.cpp src/gl/Module.cpp src/gl/ParamSet.cpp src/KeyboardManager.cpp src/Resolution.cpp src/VideoWriter.cpp src/Controller.cpp src/mathutil.cpp src/fileutil.cpp src/LuaFrontend.cpp src/Webcam.cpp)

target_compile_options(${PROJECT_NAME} PRIVATE "-Wextra" "-Wall")

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/third-party)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENGL_INCLUDE_DIR})

# OpenCV
find_package(OpenCV REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# OpenGL - GLFW
find_package(glfw3 3.2 REQUIRED)
target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES}) 

# RtMidi
find_package(RTMIDI REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${RTMIDI_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${RTMIDI_LIBRARIES})

# SFML - Audio
find_package(SFML COMPONENTS audio REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${SFML_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} sfml-audio)

# Lua
find_package(Lua REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${LUA_LIBRARY})

# LibLo
find_package(LIBLO REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBLO_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${LIBLO_LDFLAGS})

# Boost
# sudo apt-get install libboost-signals-dev libboost-filesystem-dev libboost-system-dev
find_package(Boost COMPONENTS REQUIRED filesystem system)
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

# Libraries brought in by conan dependency manager
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})

# Eliminate: /usr/local/lib/liblua.a(loadlib.o): undefined reference to symbol 'dlclose@@GLIBC_2.2.5' 
target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})

if (APPLE)
  target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()

#
##
## Testing!
##
#enable_testing()
#
#add_executable(tests test/main.cpp)
#include(GoogleTest)
#gtest_discover_tests(tests)
#target_compile_options(tests PRIVATE "-Wextra" "-Wall")
#target_link_libraries(tests ${CONAN_LIBS})
#
#
# Enable coverage
#
if (COVERAGE)
    target_link_libraries(${PROJECT_NAME} --coverage)
    #    target_link_libraries(tests --coverage)
endif()
