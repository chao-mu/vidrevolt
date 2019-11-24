#include "gl/Texture.h"

namespace vidrevolt {
    namespace gl {
        Texture::Texture() {
            GLCall(glGenTextures(1, &glID_));

            borrowBind([this]() {
                GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
                GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

                setScaleFilter(GL_LINEAR, GL_LINEAR);
            });
        }

        Texture::~Texture() {
            glDeleteTextures(1, &glID_);
        }

        void Texture::setScaleFilter(GLint min_param, GLint mag_param) {
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_param));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_param));
        }

        Resolution Texture::getResolution() {
            if (res_.width == 0) {
                borrowBind([this]() {
                    GLCall(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &res_.width));
                    GLCall(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &res_.height));
                });

            }
            return res_;
        }

        cv::Mat Texture::read() {
            cv::Mat image;
            borrowBind([&image, this]() {
                GLint alignment;
                GLCall(glGetIntegerv(GL_PACK_ALIGNMENT, &alignment));

                Resolution res = getResolution();
                int width = res.width;
                int height = res.height;

                // Load the actual image daata
                char* data = new char[static_cast<size_t>(width) * static_cast<size_t>(height) * 3];
                GLCall(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, data));

                image = cv::Mat(height, width, CV_8UC3, data);
            });

            return image;
        }

        void Texture::populate(cv::Mat& frame) {
            cv::Size size = frame.size();

            this->populate(GL_RGB, size.width, size.height, GL_RGB, GL_UNSIGNED_BYTE, frame.data);
        }

        void Texture::populate(GLint internal_format, GLsizei width, GLsizei height,
                GLenum format, GLenum type, const GLvoid* data) {

            borrowBind([internal_format, width, height, format, type, data]() {
                GLCall(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, data));
            });
        }

        void Texture::borrowBind(std::function<void()> f) {
            // Lookup current active texture so we can restore.
            GLint prev_active = 0;
            GLint prev_tex = 0;

            GLCall(glGetIntegerv(GL_ACTIVE_TEXTURE, &prev_active));
            GLCall(glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_tex));

            this->bind();

            f();

            // Restore active texture
            GLCall(glActiveTexture(prev_active));
            GLCall(glBindTexture(GL_TEXTURE_2D,  static_cast<GLuint>(prev_tex)));
            GLCall(glActiveTexture(prev_active));
        }

        void Texture::bind(unsigned int slot) {
            GLCall(glActiveTexture(GL_TEXTURE0 + slot));
            GLCall(glBindTexture(GL_TEXTURE_2D,  glID_));
        }

        void Texture::unbind() {
            GLCall(glBindTexture(GL_TEXTURE_2D, 0));
        }

        GLuint Texture::getID() const {
            return glID_;
        }
    }
}
