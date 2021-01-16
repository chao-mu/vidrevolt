#include "gl/Renderer.h"

#include <stdexcept>

#define AUX_OUTPUT_NAME "aux"

namespace vidrevolt {
    namespace gl {
        void Renderer::setResolution(const Resolution& res) {
            resolution_ = res;
        }

        void Renderer::render(const Address target, const std::string& shader_path, ParamSet params) {
            preloadModule(shader_path);

            auto res = getResolution();
            if (render_outs_.count(target) <= 0) {
                auto render = std::make_shared<RenderOut>(
                        res, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1);

                render->load();
                render_outs_[target] = render;
            }

            auto& mod = modules_.at(shader_path);
            auto program = mod->getShaderProgram();
            unsigned int slot = 0;
            auto out = render_outs_.at(target);
            out->bind(program);
            for (const auto& kv : mod->getNeeds(params)) {
                const std::string& uni_name = kv.first;
                AddressOrValue addr_or_val = kv.second;

                if (isValue(addr_or_val)) {
                    program->setUniform(uni_name, std::get<Value>(addr_or_val));
                } else if (isAddress(addr_or_val)) {
                    auto addr = std::get<Address>(addr_or_val);
                    if (textures_.count(addr) > 0) {
                        auto tex = textures_.at(addr);

                        program->setUniform(uni_name, [tex, &slot](GLint& id) {
                            tex->bind(slot);
                            glUniform1i(id, slot);
                            slot++;
                        });
                    } else if (textures_.count(addr.withoutBack()) && addr.getBack() == "resolution") {
                        auto res = textures_.at(addr.withoutBack())->getResolution();
                        program->setUniform(uni_name, [&res](GLint& id) {
                            glUniform2f(
                                id,
                                static_cast<float>(res.width),
                                static_cast<float>(res.height)
                            );
                        });
                    } else {
                        std::cerr << "WARNING: undefined texture '" << addr.str() << "' referenced"<< std::endl;
                    }
                }
            }

            /*
            program->setUniform("firstPass", [this](GLint& id) {
                glUniform1i(id, static_cast<int>(first_pass_));
            });
            */

            double time = glfwGetTime();
            program->setUniform("iTime", [&time](GLint& id) {
                glUniform1f(id, static_cast<float>(time));
            });

            program->setUniform("iResolution", [&res](GLint& id) {
                glUniform2f(
                    id,
                    static_cast<float>(res.width),
                    static_cast<float>(res.height)
                );
            });

            /*
            program->setUniform("lastOut", [this, &slot](GLint& id) {
                getLastOutTex()->bind(slot);
                glUniform1i(id, slot);
                slot++;
            });
            */

            glViewport(0,0, res.width, res.height);

            // Draw our vertices
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // Unbind and swap output/input textures
            out->unbind(program);

            textures_[target] = out->getSrcTex();
            last_ = out;

            if (target.str() == "aux") {
                last_aux_ = out;
            }
        }

        Resolution Renderer::getResolution() const {
            if (resolution_.width <= 0 || resolution_.height <= 0) {
                throw std::runtime_error("Renderer had resolution with width or height of zero, possibly unset.");
            }

            return resolution_;
        }

        void Renderer::preloadModule(const std::string& shader_path) {
            if (modules_.count(shader_path) <= 0) {
                modules_[shader_path] = std::make_unique<Module>();
                modules_.at(shader_path)->compile(shader_path);
            }
        }

        void Renderer::render(const Address target, cv::Mat& frame) {
            if (textures_.count(target) <= 0) {
                textures_[target] = std::make_shared<Texture>();
            }

            textures_.at(target)->populate(frame);
        }

        std::map<std::string, std::shared_ptr<Module>> Renderer::getModules() {
            return modules_;
        }

        std::shared_ptr<RenderOut> Renderer::getLast() {
            return last_;
        }

        std::shared_ptr<RenderOut> Renderer::getLastAux() {
            return last_aux_;
        }
    }
}
