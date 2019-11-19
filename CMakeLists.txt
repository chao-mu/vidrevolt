cmake_minimum_required (VERSION 3.10)
project (vidrevolt)

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
    NAME jinncrafters
    URL https://api.bintray.com/conan/jinncrafters/conan
)

conan_add_remote(
    NAME DEGoodmanWilson
    URL https://api.bintray.com/conan/degoodmanwilson/opensource
)

set (CONAN_SETTINGS SETTINGS compiler.libcxx=libstdc++11)
conan_cmake_run(
    REQUIRES
        inja/[~=1.0]@DEGoodmanWilson/stable
        boost_system/1.69.0@bincrafters/stable
        boost_filesystem/1.69.0@bincrafters/stable
        boost_signals2/1.69.0@bincrafters/stable
        opencv/4.1.1@conan/stable
        bzip2/1.0.8@conan/stable
        yaml-cpp/0.6.2@bincrafters/stable
        sfml/2.5.1@bincrafters/stable
        sol2/2.20.6@jinncrafters/stable
        tclap/1.2.2@vidrevolt/stable
        liblo/0.30@vidrevolt/stable
        libalsa/1.1.9@conan/stable
    BASIC_SETUP
        ${CONAN_SETTINGS}
    OPTIONS 
        opencv:dc1394=False
        opencv:protobuf=False
        opencv:harfbuzz=False
        opencv:eigen=False
        opencv:gflags=False
        opencv:quirc=False
        opencv:freetype=False   
        opencv:glog=False   
        opencv:gflags=False   
        # sudo apt-get install libva-dev  libvdpau-dev libpulse-dev libxcb1-dev libxcb-shm0-dev libxcb-shape0-dev libxcb-xfixes0-dev
        opencv:ffmpeg=True   
        sfml:graphics=False 
        sfml:window=False
        sfml:audio=True 
        sfml:network=False

    BUILD missing
)

#
# Commmon
#
add_library(common common/Keyboard.cpp common/BPMSync.cpp common/AddressOrValue.cpp common/Video.cpp common/midi/Device.cpp common/midi/Message.cpp common/midi/Control.cpp common/Image.cpp common/osc/Server.cpp common/Pipeline.cpp common/Value.cpp common/Address.cpp common/gl/Texture.cpp common/gl/GLUtil.cpp common/gl/ShaderProgram.cpp common/gl/RenderOut.cpp common/gl/IndexBuffer.cpp common/gl/Renderer.cpp common/gl/VertexArray.cpp common/gl/VertexBuffer.cpp common/gl/Module.cpp common/gl/ParamSet.cpp common/KeyboardManager.cpp common/Resolution.cpp common/RenderStep.cpp common/VideoWriter.cpp common/Controller.cpp common/mathutil.cpp common/fileutil.cpp) 

target_compile_options(common PRIVATE "-Wextra" "-Wall")

target_include_directories(common PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/common)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(common ${OPENGL_LIBRARIES})
target_link_libraries(common ${OPENGL_INCLUDE_DIRS})

# OpenGL - GLFW
find_package(glfw3 3.2 REQUIRED)
target_link_libraries(common glfw ${GLFW_LIBRARIES}) 

# OpenGL - GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
target_link_libraries(common ${GLEW_LIBRARIES})

# RtMidi
find_package(RTMIDI REQUIRED)
include_directories(${RTMIDI_INCLUDE_DIRS})
target_link_libraries(common ${RTMIDI_LIBRARIES})

# Lua
find_package(Lua REQUIRED)
include_directories(${LUA_INCLUDE_DIRS})
target_link_libraries(common ${LUA_LIBRARIES})

# Libraries brought in by conan dependency manager
target_link_libraries(common ${CONAN_LIBS})

#
# Main executable
#
add_executable(${PROJECT_NAME} vidrevolt/main.cpp)

target_compile_options(${PROJECT_NAME} PRIVATE "-Wextra" "-Wall")

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(${PROJECT_NAME} common)

# Libraries brought in by conan dependency manager
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})
