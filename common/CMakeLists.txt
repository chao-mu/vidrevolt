set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenGL_GL_PREFERENCE GLVND)

# Generate with: cd common; find -name "*.cpp" | xargs
add_library(common ./Keyboard.cpp ./BPMSync.cpp ./AddressOrValue.cpp ./Video.cpp ./midi/Device.cpp ./midi/Message.cpp ./midi/Control.cpp ./Image.cpp ./osc/Server.cpp ./Value.cpp ./Address.cpp ./Patch.cpp ./gl/Texture.cpp ./gl/GLUtil.cpp ./gl/ShaderProgram.cpp ./gl/RenderOut.cpp ./gl/IndexBuffer.cpp ./gl/VertexArray.cpp ./gl/VertexBuffer.cpp ./gl/Renderer.cpp ./gl/Module.cpp ./KeyboardManager.cpp ./Media.cpp ./Resolution.cpp ./RenderStep.cpp ./VideoWriter.cpp ./Controller.cpp ./mathutil.cpp ./fileutil.cpp ./lua/HasLuaState.cpp ./lua/Group.cpp ./lua/Controller.cpp) 

target_compile_options(common PRIVATE "-Wextra" "-Werror" "-Wall" "-pedantic-errors")

target_include_directories(common PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Boost components support
#find_package(Boost COMPONENTS system filesystem REQUIRED)
#target_link_libraries(common 
#	${Boost_FILESYSTEM_LIBRARY}
#	${Boost_SYSTEM_LIBRARY}
#)

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

# OpenCV
find_package(OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )
target_link_libraries(common ${OpenCV_LIBS})

# yaml-cpp
find_package(yaml-cpp REQUIRED)
include_directories(${YAML_CPP_INCLUDE_DIRS})
target_link_libraries(common ${YAML_CPP_LIBRARIES})

# RtMidi
find_package(RTMIDI REQUIRED)
include_directories(${RTMIDI_INCLUDE_DIRS})
target_link_libraries(common ${RTMIDI_LIBRARIES})

# SFML
find_package(SFML 2.5 COMPONENTS audio REQUIRED)
target_link_libraries(common sfml-audio)

# Liblo - OSC library
find_package(LIBLO REQUIRED)
target_link_libraries(common ${LIBLO_LIBRARIES})
include_directories(common ${LIBLO_INCLUDE_DIRS})

# Lua
find_package(Lua REQUIRED)
include_directories(${LUA_INCLUDE_DIRS})
target_link_libraries(common ${LUA_LIBRARIES})

# sol (Lua bindings)
find_package(Sol2 REQUIRED)
include_directories(${SOL2_INCLUDE_DIRS})
target_link_libraries(common ${SOL2_LIBRARIES})

# Libraries brought in by conan dependency manager
target_link_libraries(common ${CONAN_LIBS})
