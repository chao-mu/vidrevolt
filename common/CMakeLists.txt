set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenGL_GL_PREFERENCE GLVND)

# Generate with: cd common; find -name "*.cpp" | xargs
add_library(common ./Keyboard.cpp ./BPMSync.cpp ./AddressOrValue.cpp ./Video.cpp ./midi/Device.cpp ./midi/Message.cpp ./midi/Control.cpp ./Image.cpp ./osc/Server.cpp ./Value.cpp ./Address.cpp ./Pipeline.cpp ./gl/Texture.cpp ./gl/GLUtil.cpp ./gl/ShaderProgram.cpp ./gl/RenderOut.cpp ./gl/IndexBuffer.cpp ./gl/VertexArray.cpp ./gl/VertexBuffer.cpp ./gl/Renderer.cpp ./gl/Module.cpp ./KeyboardManager.cpp ./Resolution.cpp ./RenderStep.cpp ./VideoWriter.cpp ./Controller.cpp ./mathutil.cpp ./fileutil.cpp) 

target_compile_options(common PRIVATE "-Wextra" "-Werror" "-Wall" "-pedantic-errors")

target_include_directories(common PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

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
