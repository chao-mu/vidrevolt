#include "Texture.h"

namespace vidrevolt {
    Texture::Texture() {
        GLCall(glGenTextures(1, &glID_));

        GLCall(glBindTexture(GL_TEXTURE_2D, glID_));

        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

        setScaleFilter(GL_LINEAR, GL_LINEAR);

        GLCall(glBindTexture(GL_TEXTURE_2D, 0));
    }

    Texture::~Texture() {
        glDeleteTextures(1, &glID_);
    }

    void Texture::setScaleFilter(GLint min_param, GLint mag_param) {
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_param));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_param));
    }

    Resolution Texture::getResolution() {
        return res_;
    }

    void Texture::save(const std::string& path) {
        bind();

        GLint alignment;
        GLCall(glGetIntegerv(GL_PACK_ALIGNMENT, &alignment));

        Resolution res = getResolution();
        int width = res.width;
        int height = res.height;

        // Load the actual image daata
        char* data = new char[width * height * 3];
        GLCall(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, data));

        cv::Mat image(height, width, CV_8UC3, data);

        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
        flip(image, image, 0);
        cv::imwrite(path, image);

        unbind();
    }

    void Texture::populate(cv::Mat& frame) {
        cv::Size size = frame.size();

        this->populate(GL_RGB, size.width, size.height, GL_RGB, GL_UNSIGNED_BYTE, frame.data);
    }

    void Texture::populate(GLint internal_format, GLsizei width, GLsizei height,
            GLenum format, GLenum type, const GLvoid* data) {

        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, data));

        GLCall(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &res_.width));
        GLCall(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &res_.height));
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
