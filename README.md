# vidrevolt
Text driven performance tool for VJs and Visual Artists

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

## Patch Sections

### resolution

"resolution" represents your output's resolution. It has two keys, "width" and "height".

```yaml
resolution:
    width: 1920
    height: 1080
```

### vars

"vars" is a dictionary that presents a collection of variables. Variables can be thought of a mapping of an address to different address or value. The address or value the variable points to can then can be overwritten with commands.

It is a dictionary whose keys are addresses and whose values are either an address or a value.

On the left is the variable's address and on the right is the address or value referencing the variable will in turn reference.

```yaml
vars:
    clip-destination: scene.a
    main-color: [1, 0, 0]
```

### render

"render" is a list of steps that define the rendering process, referred to as "render steps". This is where you would apply effects and mix media. The final render step outputs to the screen.

Each step defines what destination address the step will render to and the details of what is being rendered and how. 

## Contributing

Right now the biggest thing we need is people using it, finding pain points, and filing tickets on github for bugs and feature requests.

Hashtag your posts #vidrevolt

### Developer Tools

* https://github.com/include-what-you-use/include-what-you-use (Unused Include Files)



