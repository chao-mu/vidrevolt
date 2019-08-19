cmake_minimum_required (VERSION 3.10)
project (vidrevolt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# My stuff
add_executable(${PROJECT_NAME} src/gl/module.cpp src/gl/RenderOut.cpp src/ValueStore.cpp src/Keyboard.cpp src/main.cpp src/PatchParser.cpp src/Parser.cpp src/AddressOrValue.cpp src/Video.cpp src/midi/Device.cpp src/midi/Message.cpp src/midi/Control.cpp src/Image.cpp src/Value.cpp src/Address.cpp src/MathUtil.cpp src/gl/Texture.cpp src/gl/GLUtil.cpp src/gl/ShaderProgram.cpp src/gl/RenderPipeline.cpp src/gl/PingPongTexture.cpp src/gl/IndexBuffer.cpp src/gl/VertexArray.cpp src/gl/VertexBuffer.cpp src/Group.cpp src/cmd/OverwriteGroup.cpp src/cmd/Command.cpp src/cmd/Rotate.cpp src/cmd/Reverse.cpp src/cmd/OverwriteVar.cpp src/Media.cpp src/Controller.cpp src/Module.cpp src/fileutil.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(${PROJECT_NAME} PRIVATE "-Wextra" "-Werror" "-Wall" "-pedantic-errors" "-Wconversion")

# Boost components support
find_package(Boost COMPONENTS system filesystem REQUIRED)
target_link_libraries(${PROJECT_NAME} 
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${OPENGL_INCLUDE_DIRS})

# OpenGL - GLFW
find_package(glfw3 3.2 REQUIRED)
target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES}) 

# OpenGL - GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})

# TCLAP
find_package(TCLAP REQUIRED)
include_directories(${TCLAP_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${TCLAP_LIBRARIES})

# OpenCV
find_package(OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# yaml-cpp
find_package(yaml-cpp REQUIRED)
include_directories(${YAML_CPP_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${YAML_CPP_LIBRARIES})

# LibAV
find_package(LIBAV REQUIRED)
include_directories(${LIBAV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${LIBAV_LIBRARIES})

# RtMidi
find_package(RTMIDI REQUIRED)
include_directories(${RTMIDI_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${RTMIDI_LIBRARIES})

# SFML
find_package(SFML 2.5 COMPONENTS audio REQUIRED)
target_link_libraries(${PROJECT_NAME} sfml-audio)
