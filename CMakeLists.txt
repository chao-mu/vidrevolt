cmake_minimum_required (VERSION 3.10)
project (vidrevolt-global)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
# Sub projects
#
add_subdirectory(common)
add_subdirectory(vidrevolt)
#add_subdirectory(exp)
#add_subdirectory(textexp)
#add_subdirectory(vidrevolt-pre)
#add_subdirectory(videxp)
#add_subdirectory(videxp2)
