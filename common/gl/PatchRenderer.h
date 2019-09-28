#ifndef VIDREVOLT_GL_PATCHRENDERER_H_
#define VIDREVOLT_GL_PATCHRENDERER_H_

// STL
#include <memory>
#include <vector>
#include <map>

// Ours (OpenGL)
#include "GLUtil.h"
#include "RenderOut.h"
#include "module.h"

// Ours
#include "../Media.h"
#include "../RenderStep.h"
#include "../Resolution.h"
#include "../Patch.h"

namespace vidrevolt {
    namespace gl {
        class PatchRenderer {
            public:
                PatchRenderer(std::shared_ptr<Patch> patch);

                std::shared_ptr<Texture> getLastOutTex();
                GLuint getFBO();
                GLuint getReadableBuf();

                void load();
                void render();
                void reload();

            private:
                std::shared_ptr<RenderOut> getLast();

                std::vector<std::shared_ptr<RenderStep>> modules_;

                std::map<std::string, std::shared_ptr<Texture>> textures_;
                std::map<std::string, std::shared_ptr<RenderOut>> render_outs_;
                std::vector<module::UniformNeeds> uniform_needs_;
                std::vector<std::shared_ptr<ShaderProgram>> programs_;
                std::shared_ptr<Patch> patch_;
                const Resolution resolution_;

                bool first_pass_ = true;
        };
    }
}

#endif

