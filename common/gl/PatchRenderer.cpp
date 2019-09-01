#include "PatchRenderer.h"

#include <stdexcept>

namespace vidrevolt {
    namespace gl {
        PatchRenderer::PatchRenderer(std::shared_ptr<Patch> patch) : patch_(patch), resolution_(patch->getResolution()) {}

        void PatchRenderer::load() {
            for (const auto& mod : patch_->getModules()) {
                std::pair<std::shared_ptr<ShaderProgram>, module::UniformNeeds> kv =
                    module::compile(mod->getPath(), mod->getParams(), patch_);

                programs_.push_back(kv.first);
                uniform_needs_.push_back(kv.second);

                std::string out_name = mod->getOutput();
                if (render_outs_.count(out_name) <= 0) {
                    auto render = std::make_shared<RenderOut>(
                            resolution_, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1);

                    render->load();
                    render_outs_[out_name] = render;
                }
            }
        }

        std::shared_ptr<RenderOut> PatchRenderer::getLast() {
            return render_outs_.at(patch_->getModules().back()->getOutput());
        }

        std::shared_ptr<Texture> PatchRenderer::getLastOutTex() {
            return getLast()->getSrcTex();
        }

        GLuint PatchRenderer::getFBO() {
            return getLast()->getFBO();
        }

        GLuint PatchRenderer::getReadableBuf() {
            return getLast()->getSrcDrawBuf();
        }

        void PatchRenderer::render() {
            patch_->startRender();

            const std::vector<std::unique_ptr<Module>>& modules = patch_->getModules();
            for (size_t i = 0; i < modules.size(); i++) {
                const auto& mod = modules.at(i);
                const auto& uniforms = uniform_needs_.at(i);
                const auto& program = programs_.at(i);

                std::shared_ptr<RenderOut> out = render_outs_.at(mod->getOutput());

                out->bind(program);

                unsigned int slot = 0;
                for (const auto& kv : uniforms) {
                    const std::string& uni_name = kv.first;
                    AddressOrValue addr_or_val = kv.second;

                    if (!program->getUniformLoc(uni_name).has_value()) {
                        continue;
                    }

                    std::optional<Value> val_opt;
                    std::optional<Address> addr_opt;
                    if (isValue(addr_or_val)) {
                        program->setUniform(uni_name, std::get<Value>(addr_or_val));
                    } else if (isAddress(addr_or_val)) {
                        auto addr = std::get<Address>(addr_or_val);
                        patch_->visitReferable(addr, [this, program, uni_name, addr, &slot](Referable r) {
                            //std::cout << "uni_name=" << uni_name << " addr=" << addr.str() << std::endl;
                            if (std::holds_alternative<Value>(r)) {
                                program->setUniform(uni_name, std::get<Value>(r));
                                return;
                            }

                            // We have texture rendering to do
                            std::shared_ptr<Texture> tex;
                            if (std::holds_alternative<Media*>(r)) {
                                auto media = std::get<Media*>(r);
                                std::string out = media->getAddress().str();
                                if (textures_.count(out) <= 0) {
                                    textures_[out] = std::make_shared<Texture>();
                                }

                                tex = textures_.at(out);

                                std::optional<cv::Mat> frame_opt = media->nextFrame();
                                if (frame_opt.has_value()) {
                                    tex->populate(frame_opt.value());
                                }
                            } else if (std::holds_alternative<ModuleOutputLabel>(r)) {
                                std::string key = std::get<std::string>(r);
                                if (textures_.count(key) > 0) {
                                    tex = textures_.at(key);
                                }
                            }

                            if (tex == nullptr) {
                                throw std::runtime_error("Expected address '" + addr.str() + "' to be a texture");
                            }

                            program->setUniform(uni_name, [tex, &slot](GLint& id) {
                                tex->bind(slot);
                                glUniform1i(id, slot);
                                slot++;
                            });
                        });
                    }
                }

                program->setUniform("firstPass", [this](GLint& id) {
                    glUniform1i(id, static_cast<int>(first_pass_));
                });

                program->setUniform("iTime", [](GLint& id) {
                    glUniform1f(id, static_cast<float>(glfwGetTime()));
                });

                program->setUniform("iResolution", [this](GLint& id) {
                    glUniform2f(
                        id,
                        static_cast<float>(resolution_.width),
                        static_cast<float>(resolution_.height)
                    );
                });

                program->setUniform("lastOut", [this, &slot](GLint& id) {
                    getLastOutTex()->bind(slot);
                    glUniform1i(id, slot);
                    slot++;
                });

                glViewport(0,0, resolution_.width, resolution_.height);

                // Draw our vertices
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                // Unbind and swap output/input textures
                out->unbind(program);

                textures_[mod->getOutput()] = out->getSrcTex();
            }

            first_pass_ = false;

            patch_->endRender();
        }
    }
}