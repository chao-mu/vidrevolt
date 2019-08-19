#include "RenderPipeline.h"

#include "../Address.h"

namespace vidrevolt {
    namespace gl {
        RenderPipeline::RenderPipeline(
                const Resolution& resolution,
                std::shared_ptr<ValueStore> store,
                std::vector<std::shared_ptr<Module>> steps) : resolution_(resolution), store_(store), steps_(steps) {}


        void RenderPipeline::load() {
            // Initialize values
            for (const auto& step : steps_) {
                step->compile(store_);
                const std::string out_name = step->getOutput();
                store_->set(Address(out_name), step->getLastOutTex());
            }

            media_ = store_->getMediaAll();
        }

        std::shared_ptr<Module> RenderPipeline::getLastStep() {
            return steps_.back();
        }

        void RenderPipeline::render() {
            for (const auto& kv : media_) {
                kv.second->resetInUse();
            }

            for (auto& step : steps_) {
                step->bind();
                step->setValues(store_, first_pass_);

                glViewport(0,0, resolution_.width, resolution_.height);

                // Draw our vertices
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                // Unbind and swap output/input textures
                step->unbind();

                store_->set(Address(step->getOutput()), step->getLastOutTex());
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
