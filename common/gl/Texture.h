#ifndef VIDREVOLT_GL_TEXTURE_H_
#define VIDREVOLT_GL_TEXTURE_H_

// STL
#include <functional>

// OpenCV
#include <opencv2/opencv.hpp>

// Ours
#include "GLUtil.h"
#include "../Resolution.h"

namespace vidrevolt {
    namespace gl {
        class Texture {
            public:
                Texture();
                ~Texture();

                cv::Mat read();
                void bind(unsigned int slot = 0);
                void unbind();
                void populate(GLint internal_format, GLsizei width, GLsizei height,
                        GLenum format, GLenum type, const GLvoid * data);
                void populate(cv::Mat& frame);
                void borrowBind(std::function<void()> f);
                void setScaleFilter(GLint min_param, GLint mag_param);
                Resolution getResolution();

                GLuint getID() const;

            private:
                unsigned int glID_ = 0;
                Resolution res_;
        };
    }
}

#endif
