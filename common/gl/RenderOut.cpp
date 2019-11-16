#include "gl/RenderOut.h"

#define SRC 0
#define DEST 1

namespace vidrevolt {
    namespace gl {
        RenderOut::RenderOut(const Resolution& res, GLenum src, GLenum dest) : resolution_(res) {
            textures_[SRC] = std::make_shared<Texture>();
            textures_[DEST] = std::make_shared<Texture>();

            draw_bufs_[SRC] = src;
            draw_bufs_[DEST] = dest;
        }

        GLuint RenderOut::getFBO() {
            return fbo_;
        }

        void RenderOut::load() {
            for (const auto& tex : textures_) {
                tex->bind();
                GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, resolution_.width, resolution_.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));
                tex->unbind();
            }

            // Create and bind the frame buffer we will be rendering to
            GLCall(glGenFramebuffers(1, &fbo_));

            // Bind the FBO in order to then associate texture's with it
            GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));

            // Bind the textures to the frame buffer
            GLCall(glFramebufferTexture(
                GL_FRAMEBUFFER,
                getSrcDrawBuf(),
                getSrcTex()->getID(),
                0
            ));

            GLCall(glFramebufferTexture(
                GL_FRAMEBUFFER,
                getDestDrawBuf(),
                getDestTex()->getID(),
                0
            ));
        }

        void RenderOut::swap() {
            std::swap(draw_bufs_[SRC], draw_bufs_[DEST]);
            std::swap(textures_[SRC], textures_[DEST]);
        }

        std::shared_ptr<Texture> RenderOut::getSrcTex() {
            return textures_[SRC];
        }

        std::shared_ptr<Texture> RenderOut::getDestTex() {
            return textures_[DEST];
        }

        GLuint RenderOut::getSrcDrawBuf() const {
            return draw_bufs_[SRC];
        }

        GLuint RenderOut::getDestDrawBuf() const {
            return draw_bufs_[DEST];
        }

        void RenderOut::bind(std::shared_ptr<ShaderProgram> program) {
            // Create and bind the frame buffer we will be rendering to
            GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
            program->bind();
            GLCall(glDrawBuffer(getDestDrawBuf()));
        }

        void RenderOut::unbind(std::shared_ptr<ShaderProgram> program) {
            program->unbind();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            swap();
        }
    }
}
