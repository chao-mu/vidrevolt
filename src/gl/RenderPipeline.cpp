#include "RenderPipeline.h"

#include "../Address.h"

namespace vidrevolt {
    namespace gl {
        RenderPipeline::RenderPipeline(
                const Resolution& resolution,
                std::shared_ptr<ValueStore> store,
                std::vector<std::shared_ptr<Module>> modules) : resolution_(resolution), store_(store), modules_(modules) {}


        void RenderPipeline::load() {
            for (const auto& mod : modules_) {
                std::pair<std::shared_ptr<ShaderProgram>, module::UniformNeeds> kv =
                    module::compile(mod->getPath(), mod->getParams(), store_);

                programs_.push_back(kv.first);
                uniform_needs_.push_back(kv.second);

                std::string out_name = mod->getOutput();
                if (render_outs_.count(out_name) <= 0) {
                    auto render = std::make_shared<RenderOut>(
                            resolution_, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1);

                    render->load();
                    render_outs_[out_name] = render;

                    store_->set(Address(out_name), render->getSrcTex());
                }
            }

            media_ = store_->getMediaAll();
        }

        std::shared_ptr<RenderOut> RenderPipeline::getLast() {
            return render_outs_.at(modules_.back()->getOutput());
        }

        std::shared_ptr<Texture> RenderPipeline::getLastOutTex() {
            return getLast()->getSrcTex();
        }

        GLuint RenderPipeline::getFBO() {
            return getLast()->getFBO();
        }

        GLuint RenderPipeline::getReadableBuf() {
            return getLast()->getSrcDrawBuf();
        }

        void RenderPipeline::render() {
            for (const auto& kv : media_) {
                kv.second->resetInUse();
            }

            for (size_t i = 0; i < modules_.size(); i++) {
                const auto& mod = modules_.at(i);
                const auto& uniforms = uniform_needs_.at(i);
                const auto& program = programs_.at(i);

                std::shared_ptr<RenderOut> out = render_outs_.at(mod->getOutput());

                out->bind(program);

                int slot = 0;
                for (const auto& kv : uniforms) {
                    const std::string& uni_name = kv.first;
                    AddressOrValue addr_or_val = kv.second;

                    std::optional<Value> val_opt;
                    std::optional<Address> addr_opt;
                    if (isValue(addr_or_val)) {
                        val_opt = std::get<Value>(addr_or_val);
                    } else if (isAddress(addr_or_val)) {
                        addr_opt = std::get<Address>(addr_or_val);
                        val_opt = store_->getValue(addr_opt.value());
                    }

                    if (val_opt.has_value()) {
                        Value val = val_opt.value();
                        program->setUniform(uni_name, val);
                    } else if (addr_opt.has_value() && store_->isMedia(addr_opt.value())) {
                        std::shared_ptr<Media> tex = store_->getMedia(addr_opt.value());
                        program->setUniform(uni_name, [&tex, &slot](GLint& id) {
                            tex->bind(slot);
                            tex->update();
                            glUniform1i(id, slot);
                            slot++;
                        });
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
                }

                glViewport(0,0, resolution_.width, resolution_.height);

                // Draw our vertices
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                // Unbind and swap output/input textures
                out->unbind(program);

                store_->set(Address(mod->getOutput()), out->getSrcTex());
            }

            for (const auto& kv : media_) {
                const auto& m = kv.second;
                if (m->wasInUse() && !m->isInUse()) {
                    m->outFocus();
                } else if (!m->wasInUse() && m->isInUse()) {
                    m->inFocus();
                }
            }

            first_pass_ = false;
        }
    }
}
