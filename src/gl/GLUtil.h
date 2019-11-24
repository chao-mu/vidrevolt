#ifndef VIDREVOLT_GL_GLUTIL_H_
#define VIDREVOLT_GL_GLUTIL_H_

// STL
#include <cstdlib>
#include <stdexcept>

// OpenGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLCall(x) \
    while (glGetError() != GL_NO_ERROR); \
    x; \
    if (!vidrevolt::gl::GLLogCall(#x, __FILE__, __LINE__)) throw std::runtime_error("OpenGL Error");

    //if (!GLLogCall(#x, __FILE__, __LINE__)) exit(EXIT_FAILURE);


namespace vidrevolt {
    namespace gl {
        bool GLLogCall(const char* function, const char* file, int line);
    }
}

#endif
