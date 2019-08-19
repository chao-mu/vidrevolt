#include "Module.h"

// STL
#include <sstream>
#include <regex>

// Ours
#include "fileutil.h"
#include "gl/module.h"

namespace vidrevolt {
    Module::Module(
            const std::string& output,
            const std::string& path,
            const Resolution& res
        ) : output_(output), path_(path), resolution_(res) , program_(std::make_shared<gl::ShaderProgram>()) {

        // Our render target
        render_out_ = std::make_shared<gl::RenderOut>(
                res,
                GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1);

        // TODO: Move - can throw
        render_out_->load();
    }

    Resolution Module::getResolution() {
        return resolution_;
    }

    std::string Module::toInputName(const std::string& name) {
        return gl::module::toInternalName(name);
    }

    GLenum Module::getReadableBuf() {
        return render_out_->getSrcDrawBuf();
    }

    GLuint Module::getFBO() {
        return render_out_->getFBO();
    }

    std::shared_ptr<gl::Texture> Module::getLastOutTex() {
        return render_out_->getSrcTex();
    }

    void Module::unbind() {
        render_out_->unbind(program_);
    }

    void Module::bind() {
        render_out_->bind(program_);
    }

    void Module::setParam(const std::string& name, Param p) {
        params_[name].value = p.value;
        params_[name].amp = p.amp;
        params_[name].shift = p.shift;
    }

    const std::string& Module::getOutput() const {
        return output_;
    }

    const std::string& Module::getPath() const {
        return path_;
    }

    std::shared_ptr<gl::ShaderProgram> Module::getShaderProgram() {
        return program_;
    }

    void Module::compile(std::shared_ptr<ValueStore> store) {
        std::pair<std::shared_ptr<gl::ShaderProgram>, gl::module::UniformNeeds> kv =
            gl::module::compile(path_, params_, store);

        program_ = kv.first;
        uniforms_ = kv.second;
    }

    void Module::setValues(std::shared_ptr<ValueStore> store, bool first_pass) {
        unsigned int slot = 0;

        for (const auto& kv : uniforms_) {
            const std::string& uni_name = kv.first;
            AddressOrValue addr_or_val = kv.second;

            std::optional<Value> val_opt;
            std::optional<Address> addr_opt;
            if (isValue(addr_or_val)) {
                val_opt = std::get<Value>(addr_or_val);
            } else if (isAddress(addr_or_val)) {
                addr_opt = std::get<Address>(addr_or_val);
                val_opt = store->getValue(addr_opt.value());
            }

            if (val_opt.has_value()) {
                Value val = val_opt.value();
                program_->setUniform(uni_name, val);
            } else if (addr_opt.has_value() && store->isMedia(addr_opt.value())) {
                std::shared_ptr<Media> tex = store->getMedia(addr_opt.value());
                program_->setUniform(uni_name, [&tex, &slot](GLint& id) {
                    tex->bind(slot);
                    tex->update();
                    glUniform1i(id, slot);
                    slot++;
                });
            }
        }

        program_->setUniform("firstPass", [first_pass](GLint& id) {
            glUniform1i(id, static_cast<int>(first_pass));
        });


        program_->setUniform("iTime", [](GLint& id) {
            glUniform1f(id, static_cast<float>(glfwGetTime()));
        });

        program_->setUniform("iResolution", [this](GLint& id) {
            glUniform2f(
                id,
                static_cast<float>(resolution_.width),
                static_cast<float>(resolution_.height)
            );
        });

        program_->setUniform("lastOut", [this, &slot](GLint& id) {
            getLastOutTex()->bind(slot);
            glUniform1i(id, slot);
            slot++;
        });
    }
}
