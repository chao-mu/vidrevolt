#ifndef VIDREVOLT_GL_RENDERPIPELINE_H_
#define VIDREVOLT_GL_RENDERPIPELINE_H_

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
#include "../ValueStore.h"
#include "../Module.h"
#include "../Resolution.h"

namespace vidrevolt {
    namespace gl {
        class RenderPipeline {
            public:
                RenderPipeline(
                        const Resolution& resolution,
                        std::shared_ptr<ValueStore> store,
                        std::vector<std::shared_ptr<Module>> steps);

                std::shared_ptr<Texture> getLastOutTex();
                GLuint getFBO();
                GLuint getReadableBuf();

                void load();
                void render();

            private:
                std::shared_ptr<RenderOut> getLast();

                const Resolution resolution_;
                std::shared_ptr<ValueStore> store_;
                std::vector<std::shared_ptr<Module>> modules_;
                std::map<Address, std::shared_ptr<Media>> media_;

                std::map<std::string, std::shared_ptr<RenderOut>> render_outs_;
                std::vector<module::UniformNeeds> uniform_needs_;
                std::vector<std::shared_ptr<ShaderProgram>> programs_;

                bool first_pass_ = true;
        };
    }
}

#endif

