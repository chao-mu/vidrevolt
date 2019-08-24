#ifndef VIDREVOLT_GL_RENDER_OUT_H_
#define VIDREVOLT_GL_RENDER_OUT_H_

// STL
#include <memory>

// Ours
#include "Texture.h"
#include "GLUtil.h"
#include "ShaderProgram.h"

namespace vidrevolt {
    namespace gl {
        class RenderOut {
            public:
                RenderOut(const Resolution& res, GLenum src, GLenum dest);
                void load();

                std::shared_ptr<Texture> getSrcTex();
                std::shared_ptr<Texture> getDestTex();
                GLuint getSrcDrawBuf() const;
                GLuint getDestDrawBuf() const;

                void unbind(std::shared_ptr<ShaderProgram> program);
                void bind(std::shared_ptr<ShaderProgram> program);

                GLuint getFBO();

                void swap();

            protected:
                std::shared_ptr<Texture> textures_[2];
                GLuint draw_bufs_[2];
                GLuint fbo_;
                const Resolution resolution_;
        };
    }
}

#endif

