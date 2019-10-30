#ifndef VIDREVOLT_GL_ENGINE_H_
#define VIDREVOLT_GL_ENGINE_H_

// STL
#include <string>
#include <variant>

// OpenCV
#include <opencv2/opencv.hpp>

// Ours
#include "../Value.h"
#include "RenderOut.h"
#include "Module.h"
#include "ParamSet.h"
#include "../Resolution.h"

namespace vidrevolt {
    namespace gl {
        class Renderer {
            public:
                Renderer(const Resolution& resolution);

                void render(const Address target, const std::string& shader_path, ParamSet params);
                void render(const Address target, cv::Mat& frame);

                std::shared_ptr<RenderOut> getLast();

            private:
                std::map<Address, std::shared_ptr<Texture>> textures_;
                std::map<Address, std::shared_ptr<RenderOut>> render_outs_;
                std::map<std::string, std::unique_ptr<Module>> modules_;

                std::shared_ptr<RenderOut> last_;

                const Resolution resolution_;

                //bool first_pass_ = true;
        };
    }
}
#endif
