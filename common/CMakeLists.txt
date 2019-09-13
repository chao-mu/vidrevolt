# Generate with: cd common; find -name "*.cpp" | xargs
add_library(common ./VideoWriter.cpp ./Keyboard.cpp ./BPMSync.cpp ./AddressOrValue.cpp ./Video.cpp ./midi/Device.cpp ./midi/Message.cpp ./midi/Control.cpp ./Image.cpp ./Value.cpp ./Address.cpp ./PatchBuilder.cpp ./mathutil.cpp ./Patch.cpp ./gl/Texture.cpp ./gl/GLUtil.cpp ./gl/ShaderProgram.cpp ./gl/RenderOut.cpp ./gl/IndexBuffer.cpp ./gl/PatchRenderer.cpp ./gl/VertexArray.cpp ./gl/VertexBuffer.cpp ./gl/module.cpp ./KeyboardManager.cpp ./Group.cpp ./cmd/OverwriteGroup.cpp ./cmd/Command.cpp ./cmd/TapTempo.cpp ./cmd/Rotate.cpp ./cmd/Reverse.cpp ./cmd/OverwriteVar.cpp ./Media.cpp ./Resolution.cpp ./Controller.cpp ./Module.cpp ./fileutil.cpp)

target_include_directories(common PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Boost components support
find_package(Boost COMPONENTS system filesystem REQUIRED)
target_link_libraries(common 
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
)

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

# TCLAP
find_package(TCLAP REQUIRED)
include_directories(${TCLAP_INCLUDE_DIRS})
target_link_libraries(common ${TCLAP_LIBRARIES})

# OpenCV
find_package(OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )
target_link_libraries(common ${OpenCV_LIBS})

# yaml-cpp
find_package(yaml-cpp REQUIRED)
include_directories(${YAML_CPP_INCLUDE_DIRS})
target_link_libraries(common ${YAML_CPP_LIBRARIES})

# LibAV
find_package(LIBAV REQUIRED)
include_directories(${LIBAV_INCLUDE_DIRS})
target_link_libraries(common ${LIBAV_LIBRARIES})

# RtMidi
find_package(RTMIDI REQUIRED)
include_directories(${RTMIDI_INCLUDE_DIRS})
target_link_libraries(common ${RTMIDI_LIBRARIES})

# SFML
find_package(SFML 2.5 COMPONENTS audio REQUIRED)
target_link_libraries(common sfml-audio)
