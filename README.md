# vidrevolt

Lua and GLSL driven performance tool for VJs and Visual Artists

![](https://images.mentalfloss.com/sites/default/files/styles/insert_main_wide_image/public/under%20construction1_0.gif)

WARNING: This project is not currently fit for public consumption! If it interests you, shoot an email to danimal@bikegasm.org and ask about how to help.

![](https://images.mentalfloss.com/sites/default/files/styles/insert_main_wide_image/public/under%20construction1_0.gif)

## Current Users                                                     

VJ and visual artist Danimal (Hackpoetic) - https://www.instagram.com/hackpoetic                   

## Building

Dependencies include
* cmake
* Boost ("system" and "filesystem")
* OpenCV
* yaml-cpp 
* SFML
* OpenGL
* GLFW
* GLEW
* TCLAP
* RtMidi

The installation command for ubuntu looks like the following:
```
sudo apt-get install cmake libboost-filesystem1.67-dev libboost-system1.67-dev libyaml-cpp-dev libopencv-dev libsfml-audio2.5 mesa-common-dev libgl1-mesa-dev libglfw3-dev libglu1-mesa-dev libglew-dev libtclap-dev librtmidi-dev libsfml-dev
```

## Starting a Project

After building the source and having the binary available, create a directory somewhere on your system to represent your project.

In the root of it check out our sibbling shader repository in a directory called "shaders" and the sibbling repository for controlers (midi and joystick) which can be called "controllers"

```
git clone git@github.com:chao-mu/vidrevolt-shaders.git shaders
git clone git@github.com:chao-mu/vidrevolt-controllers.git controllers
```

## Contributing

Right now the biggest thing we need is people using it, finding pain points, and filing tickets on github for bugs and feature requests.

Hashtag your posts #vidrevolt

### Developer Tools

* https://github.com/include-what-you-use/include-what-you-use (Unused Include Files)
