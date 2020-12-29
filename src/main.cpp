// STL
#include <cxxabi.h>
#include <cstdio>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <future>

// OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// OpenCV
#include "opencv2/opencv.hpp"

// TCLAP
#include <tclap/ArgException.h>
#include <tclap/CmdLine.h>
#include <tclap/SwitchArg.h>
#include <tclap/ValueArg.h>

// Ours
#include "Keyboard.h"
#include "KeyboardManager.h"
#include "mathutil.h"
#include "Pipeline.h"
#include "LuaFrontend.h"
#include "debug.h"
#include "gl/GLUtil.h"
#include "gl/IndexBuffer.h"
#include "gl/Texture.h"
#include "gl/VertexArray.h"
#include "gl/VertexBuffer.h"
#include "Resolution.h"
#include "Value.h"
#include "Controller.h"
#include "VideoWriter.h"

#ifndef DOUBLE_BUF
#define DOUBLE_BUF true
#endif

#define RECORDING_FPS 30

// GLFW error callback
void onError(int errc, const char* desc) {
    std::cerr << "Error (" << std::to_string(errc) << "): " << std::string(desc) << std::endl;
}
//

//// OpenGL Debug Callback
//void onOpenGLDebug(
//        GLenum /*source*/,
//        GLenum /*type*/,
//        GLuint /*id*/,
//        GLenum /*severity*/,
//        GLsizei /*length*/,
//        const GLchar *msg,
//        const void * /*data*/) {
//
//    std::cout << "OpenGL Debug: " << msg << std::endl;
//}

// GLFW window resizing callback
void onWindowSize(GLFWwindow* /* window */, int width, int height) {
    // Resize the view port when a window resize is requested
    GLCall(glViewport(0,0, width, height));
}

int main(int argc, const char** argv) {
    TCLAP::CmdLine cmd("VidRevolt");

    TCLAP::ValueArg<std::string> vert_arg("", "vert", "path to vertex shader", false, "vert.glsl", "string", cmd);
    TCLAP::ValueArg<std::string> pipeline_arg("i", "pipeline", "path to yaml pipeline", false, "pipeline.yml", "string", cmd);
    TCLAP::ValueArg<std::string> img_out_arg("", "image-out", "output image path", false, "", "string", cmd);
    TCLAP::ValueArg<std::string> vid_out_arg("o", "vid-out", "output to video path", false, "", "string", cmd);
    TCLAP::ValueArg<int> height_arg("", "height", "window height (width will be calculated automatically)", false, 720, "int", cmd);
    TCLAP::SwitchArg debug_timer_arg("", "debug-timer", "debug time between frames", cmd);
    TCLAP::SwitchArg debug_opengl("", "debug-opengl", "print out OpenGL debugging info", cmd);
    TCLAP::SwitchArg full_arg("", "full", "full screen", cmd);
    TCLAP::SwitchArg hide_title_arg("", "hide-title", "hide titlebar", cmd);

    // Parse command line arguments
    try {
        cmd.parse(argc, argv);
    } catch (TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }

    bool debug_time = debug_timer_arg.getValue();

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

    if (hide_title_arg.getValue()) {
        glfwWindowHint(GLFW_DECORATED, false);
    }

    // Use single buffer rendering
    if (!DOUBLE_BUF) {
        glfwWindowHint( GLFW_DOUBLEBUFFER, GL_FALSE );
    }

    GLFWmonitor* monitor = nullptr;
    if (full_arg.getValue()) {
        monitor = glfwGetPrimaryMonitor();
    }

    GLFWwindow* window = glfwCreateWindow(1080, 720, "Awesome Art", monitor, NULL);
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
    glfwSetKeyCallback(window, vidrevolt::KeyboardManager::onKey);

    // Make the context of the specified window current for our current thread
    glfwMakeContextCurrent(window);

    // Initialize Glad
    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cerr << "failed to load glad! :-(" << std::endl;
        return 1;
    }

    /*
    if (debug_opengl.getValue()) {
        printf("OpenGL Debug: %s: \n", glGetString(GL_VERSION));

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(onOpenGLDebug, NULL);
    }
    */

    auto pipeline = std::make_shared<vidrevolt::Pipeline>();
    auto frontend = std::make_shared<vidrevolt::LuaFrontend>(pipeline_arg.getValue(), pipeline);

    try {
        frontend->load();
    } catch (const std::runtime_error& error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return 1;
    }

    const vidrevolt::Resolution resolution = pipeline->getResolution();

    // Readjust now that we were able to load the pipeline's proclaimed resolution
    auto height = static_cast<float>(height_arg.getValue());
    auto ratio = static_cast<float>(resolution.height) / height;
    auto width = static_cast<float>(resolution.width) / ratio;
    glfwSetWindowSize(window, static_cast<int>(width), static_cast<int>(height));

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

    // Keyboard mappings
    std::shared_ptr<vidrevolt::Keyboard> keyboard = vidrevolt::KeyboardManager::makeKeyboard();

    // Screenshot key
    std::string out_path = img_out_arg.getValue();
    std::vector<std::future<void>> shot_futures_;
    keyboard->connect("p", [&shot_futures_, &out_path, &frontend](vidrevolt::Value v) {
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

        cv::Mat image = frontend->render()->getSrcTex()->read();

        // Explicitly image by copy; if we pass by reference the internal refcount wont increment
        shot_futures_.push_back(std::async([dest, image]() {
            cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
            flip(image, image, 0);
            cv::imwrite(dest, image);
            std::cout << "Screenshot saved at " << dest << std::endl;
        }));
    });

    keyboard->connect("c", [pipeline](vidrevolt::Value v) {
        if (v.getBool()) {
            return;
        }

        pipeline->reconnectControllers();
    });

    // Exit key
    keyboard->connect("escape", [&window](vidrevolt::Value v) {
        // On key release
        if (v.getBool()) {
            return;
        }

        GLCall(glfwSetWindowShouldClose(window, GLFW_TRUE));
    });

    std::unique_ptr<vidrevolt::VideoWriter> writer;
    if (vid_out_arg.getValue() != "" ) {
        std::string path = vid_out_arg.getValue();

        writer = std::make_unique<vidrevolt::VideoWriter>(path, RECORDING_FPS, resolution);
        writer->start();
    }


    // Our run loop
    DEBUG_TIME_DECLARE(render)
    DEBUG_TIME_DECLARE(loop)
    DEBUG_TIME_DECLARE(draw)
    DEBUG_TIME_DECLARE(flush)

    // Force lazy-loading
    frontend->render();

    std::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> last_write;
    GLCall(glClearColor(0, 0, 0, 1));
    while (!glfwWindowShouldClose(window)) {
        DEBUG_TIME_START(loop)
        GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

        GLCall(glfwPollEvents());

        keyboard->poll();

        DEBUG_TIME_START(render)
        std::shared_ptr<vidrevolt::gl::RenderOut> out = frontend->render();
        DEBUG_TIME_END(render)

        // Calculate blit settings
        int win_width, win_height;
        GLCall(glfwGetFramebufferSize(window, &win_width, &win_height));
        auto draw_info = vidrevolt::mathutil::DrawInfo::scaleCenter(
            static_cast<float>(resolution.width),
            static_cast<float>(resolution.height),
            static_cast<float>(win_width),
            static_cast<float>(win_height)
        );

        // Draw to the screen
        DEBUG_TIME_START(draw)
        if (DOUBLE_BUF) {
            GLCall(glDrawBuffer(GL_BACK));
        }
        GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, out->getFBO()));
        GLCall(glReadBuffer(out->getSrcDrawBuf()));
        GLCall(glViewport(0,0, win_width, win_height));
        GLCall(glBlitFramebuffer(
            0,0, resolution.width, resolution.height,
            draw_info.x0, draw_info.y0, draw_info.x1, draw_info.y1,
            GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
            GL_NEAREST
        ));

        // Show buffer
        DEBUG_TIME_START(flush);
        if (DOUBLE_BUF) {
            glfwSwapBuffers(window);
        } else {
            GLCall(glFlush());
        }
        DEBUG_TIME_END(flush);
        DEBUG_TIME_END(draw);

        if (writer != nullptr) {
            bool should_write = true;
            if (last_write) {
                std::chrono::duration<double, std::milli> time_elapsed(
                        std::chrono::high_resolution_clock::now() - last_write.value());
                should_write = (1000 / RECORDING_FPS) - time_elapsed.count() <= 0;
            }

            if (should_write) {
                cv::Mat frame = out->getSrcTex()->read();
                writer->write(frame);
                last_write = std::chrono::high_resolution_clock::now();
            }
        }

        DEBUG_TIME_END(loop)
    }

    if (writer != nullptr) {
        writer->close();
    }

    return 0;
}
