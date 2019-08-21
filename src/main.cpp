// STL
#include <cxxabi.h>
#include <stdio.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Open GL
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// TCLAP
#include <tclap/ArgException.h>
#include <tclap/CmdLine.h>
#include <tclap/SwitchArg.h>
#include <tclap/ValueArg.h>

// SMFL
#include <SFML/Audio/Music.hpp>

// Ours
#include "Keyboard.h"
#include "MathUtil.h"
#include "PatchParser.h"
#include "debug.h"
#include "gl/GLUtil.h"
#include "gl/IndexBuffer.h"
#include "gl/RenderPipeline.h"
#include "gl/Texture.h"
#include "gl/VertexArray.h"
#include "gl/VertexBuffer.h"
#include "Resolution.h"
#include "Value.h"
#include "ValueStore.h"

// GLFW error callback
void onError(int errc, const char* desc) {
    std::cerr << "Error (" << std::to_string(errc) << "): " << std::string(desc) << std::endl;
}

// OpenGL Debug Callback
void onOpenGLDebug(
        GLenum /*source*/,
        GLenum /*type*/,
        GLuint /*id*/,
        GLenum /*severity*/,
        GLsizei /*length*/,
        const GLchar *msg,
        const void * /*data*/) {

    std::cout << "OpenGL Debug: " << msg << std::endl;
}


// GLFW window resizing callback
void onWindowSize(GLFWwindow* /* window */, int width, int height) {
    // Resize the view port when a window resize is requested
    GLCall(glViewport(0,0, width, height));
}

int main(int argc, const char** argv) {
    TCLAP::CmdLine cmd("VidRevolt");

    TCLAP::ValueArg<std::string> vert_arg("", "vert", "path to vertex shader", false, "vert.glsl", "string", cmd);
    TCLAP::ValueArg<std::string> patch_arg("i", "patch", "path to yaml patch", false, "patch.yml", "string", cmd);
    TCLAP::ValueArg<std::string> img_out_arg("o", "image-out", "output image path", false, "", "string", cmd);
    TCLAP::ValueArg<std::string> sound_arg("s", "sound-path", "path to sound file", false, "", "string", cmd);
    TCLAP::ValueArg<int> height_arg("", "height", "window height (width will be calculated automatically)", false, 720, "int", cmd);
    TCLAP::ValueArg<double> fps_arg("", "fps", "FPS to aim for", false, 120, "float", cmd);
    TCLAP::SwitchArg debug_timer_arg("", "debug-timer", "debug time between frames", cmd);
    TCLAP::SwitchArg debug_store_arg("", "debug-store", "print out the value store", cmd);
    TCLAP::SwitchArg debug_opengl("", "debug-opengl", "print out OpenGL debugging info", cmd);
    TCLAP::SwitchArg full_arg("", "full", "maximized, no titlebar", cmd);

    // Parse command line arguments
    try {
        cmd.parse(argc, argv);
    } catch (TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }

    bool debug_time = debug_timer_arg.getValue();

    sf::Music music;
    const std::string sound_path = sound_arg.getValue();
    if (!sound_path.empty()) {
        if (!music.openFromFile(sound_path)) {
            // openFromFile displays its own helpful error, so none here
            return 1;
        }
    }

    // Set GLFW error callback
    glfwSetErrorCallback(onError);

    // Initialize the GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!!!" << std::endl;
        return 1;
    }

    // OpenGL version and compatibility settings
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /*
    if (full_arg.getValue()) {
        glfwWindowHint(GLFW_DECORATED, false);
        glfwWindowHint(GLFW_MAXIMIZED, true);
    }
    */

    vidrevolt::PatchParser parser(patch_arg.getValue());
    const vidrevolt::Resolution resolution = parser.getResolution();


    GLFWmonitor* monitor = nullptr;
    if (full_arg.getValue()) {
        monitor = glfwGetPrimaryMonitor();
    }

    float height = static_cast<float>(height_arg.getValue());
    float ratio = static_cast<float>(resolution.height) / height;
    float width = static_cast<float>(resolution.width) / ratio;
    GLFWwindow* window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), "Awesome Art", monitor, NULL);
    if (!window) {
        glfwTerminate();
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }

    if (full_arg.getValue()) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }

    // Set window resize callback
    glfwSetWindowSizeCallback(window, onWindowSize);

    // Set key press callback
    glfwSetKeyCallback(window, vidrevolt::Keyboard::onKey);

    // Make the context of the specified window current for our current thread
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    glewInit();

    if (debug_opengl.getValue()) {
        printf("OpenGL Debug: %s: \n", glGetString(GL_VERSION));

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(onOpenGLDebug, NULL);
    }

    // Bind vertex array object
    vidrevolt::gl::VertexArray vao;
    vao.bind();

    // Copy indices into element buffer
    unsigned int indices[] = {
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    vidrevolt::gl::IndexBuffer ebo(indices, 6);
    ebo.bind();

    // Copy position vetex attributes
    GLfloat pos[] = {
        1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
    };
    vidrevolt::gl::VertexBuffer pos_vbo(pos, sizeof(pos));
    pos_vbo.bind();

    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL));
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));

    // Load all the goodies
    parser.parse();

    std::vector<std::shared_ptr<vidrevolt::Module>> modules = parser.getModules();
    if (modules.empty()) {
        std::cerr << "No modules specified. Nothing to do." << std::endl;
        return 1;
    }

    std::shared_ptr<vidrevolt::ValueStore> store = parser.getValueStore();
    auto pipeline = std::make_shared<vidrevolt::gl::RenderPipeline>(resolution, store, modules);
    pipeline->load();

    if (!sound_path.empty()) {
        music.play();
    }

    // Our run loop
    DEBUG_TIME_DECLARE(render)
    DEBUG_TIME_DECLARE(loop)
    DEBUG_TIME_DECLARE(draw)

    std::string store_str;
    if (debug_store_arg.getValue()) {
        store_str = store->str();
        std::cout << store_str << std::endl;
    }

    // Keyboard mappings
    vidrevolt::Keyboard keyboard;

    // Screenshot key
    std::string out_path = img_out_arg.getValue();
    keyboard.connect("p", [&out_path, &pipeline](vidrevolt::Value v) {
        // On key release
        if (v.getBool()) {
            return;
        }

        std::string dest = out_path;
        if (dest.empty()) {
            std::stringstream s;
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()
                    ).count();
            std::time_t now = std::time(nullptr);
            s << "output-" << std::put_time(std::localtime(&now), "%Y-%m-%d_") << ms << ".png";
            dest = s.str();
        }

        pipeline->getLastOutTex()->save(dest);

        std::cout << "Screenshot saved at " << dest << std::endl;
    });

    // Exit key
    keyboard.connect("escape", [&window](vidrevolt::Value v) {
        // On key release
        if (v.getBool()) {
            return;
        }

        GLCall(glfwSetWindowShouldClose(window, GLFW_TRUE));
    });

    while (!glfwWindowShouldClose(window)) {
        //std::chrono::time_point<std::chrono::high_resolution_clock> fps_timer_start =
        //    std::chrono::high_resolution_clock::now();

        DEBUG_TIME_START(loop)
        GLCall(glfwPollEvents());

        DEBUG_TIME_START(render)
        pipeline->render();
        DEBUG_TIME_END(render)

        // Calculate blit settings
        int win_width, win_height;
        glfwGetWindowSize(window, &win_width, &win_height);
        DrawInfo draw_info = DrawInfo::scaleCenter(
            static_cast<float>(resolution.width),
            static_cast<float>(resolution.height),
            static_cast<float>(win_width),
            static_cast<float>(win_height)
        );

        // Draw to the screen
        DEBUG_TIME_START(draw)
        glDrawBuffer(GL_BACK);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, pipeline->getFBO());
        glReadBuffer(pipeline->getReadableBuf());
        glViewport(0,0, win_width, win_height);
        glBlitFramebuffer(
            0,0, resolution.width, resolution.height,
            draw_info.x0, draw_info.y0, draw_info.x1, draw_info.y1,
            GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
            GL_NEAREST
        );
        DEBUG_TIME_END(draw)

        // Show buffer
        glfwSwapBuffers(window);

        /*
        std::chrono::duration<double, std::milli> time_elapsed(std::chrono::high_resolution_clock::now() - fps_timer_start);

        DEBUG_TIME_END(loop)

        double remainder = (1000 / fps_arg.getValue()) - time_elapsed.count();
        if (remainder < 0) {
            //std::cout << "FPS remainder: " << remainder << std::endl;
        } else {
            //std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(remainder));
        }
        */

        if (debug_store_arg.getValue()) {
            std::string ss = store->str();

            if (ss != store_str) {
                std::cout << store_str << std::endl;
                store_str = ss;
            }
        }

        DEBUG_TIME_END(loop)
    }

    return 0;
}
