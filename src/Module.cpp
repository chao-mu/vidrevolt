#include "Module.h"

// STL
#include <sstream>
#include <regex>

// Ours
#include "fileutil.h"

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
        return "se_input_" + name;
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
        const std::string& chan_name = toInputName(name);
        uniforms_[chan_name] = p.value;
        uniforms_[chan_name + "_amp"] = p.amp;
        uniforms_[chan_name + "_shift"] = p.shift;

        params_[name] = p;
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
        std::string vert_shader = R"V(
            #version 410

            out vec2 tc;
            out vec2 texcoord;
            out vec2 texcoordL;
            out vec2 texcoordR;
            out vec2 texcoordT;
            out vec2 texcoordTL;
            out vec2 texcoordTR;
            out vec2 texcoordB;
            out vec2 texcoordBL;
            out vec2 texcoordBR;
            out vec2 uv;

            uniform vec2 iResolution;

            layout (location = 0) in vec3 aPos;
        )V";

        for (const auto& kv : params_) {
            if (!isAddress(kv.second.value) || !store->isMedia(std::get<Address>(kv.second.value))) {
                continue;
            }

            const std::string& name = kv.first;
            const std::string internal_name = toInputName(name);

            vert_shader += "uniform vec2 " + internal_name + "_res;\n";
            vert_shader += "out vec2 " + internal_name + "_tc;\n";
        }

        vert_shader += R"V(
            void main() {
                gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);

                float widthStep = 1. / iResolution.x;
                float heightStep = 1. / iResolution.y;

                vec2 st = aPos.xy * .5 + .5;
                vec2 uv_in = st - .5;
                uv_in.x *= iResolution.x / iResolution.y;
                uv = uv_in;

                tc = st;
                texcoord = st;
                texcoordL = st + vec2(-widthStep, 0);
                texcoordR = st + vec2(widthStep, 0);

                texcoordT = st + vec2(0, heightStep);
                texcoordTL = st + vec2(-widthStep, heightStep);
                texcoordTR = st + vec2(widthStep, heightStep);

                texcoordB = st + vec2(0, -heightStep);
                texcoordBL = st + vec2(-widthStep, -heightStep);
                texcoordBR = st + vec2(widthStep, -heightStep);
        )V";

        for (const auto& kv : params_) {
            if (!isAddress(kv.second.value) || !store->isMedia(std::get<Address>(kv.second.value))) {
                continue;
            }

            const std::string& name = kv.first;
            const std::string internal_name = toInputName(name);
            const std::string res_name = internal_name + "_res";
            const std::string tc_name = internal_name + "_tc";

            vert_shader += tc_name + " = uv_in;\n";
            vert_shader += tc_name + ".x /= " + res_name + ".x / " + res_name + ".y;\n";
            vert_shader += tc_name + " += .5;\n";
        }

        vert_shader += "}";

        std::ifstream ifs(path_);
        if (ifs.fail()) {
            std::ostringstream err;
            err << "Error loading " << path_ << " - " <<  std::strerror(errno);
            throw std::runtime_error(err.str());
        }

        std::stringstream frag_shader;
        const std::regex pragma_input_re(R"(^#pragma\s+input\s+(.*)$)");
        const std::regex pragma_include_re(R"(^#pragma\s+include\s+(.*)\s*$)");
        const std::regex input_info_re(R"(^(\w+)\s+(\w+)\s*(.*)$)");
        std::smatch match;

        std::string line;
        int line_no = 0;
        while (std::getline(ifs, line)) {
            line_no++;

            if (std::regex_match(line, match, pragma_include_re)) {
                frag_shader << fileutil::slurp(path_, match[1]);
            } else if (std::regex_match(line, match, pragma_input_re)) {
                std::string input_info = match[1].str();
                if (std::regex_match(input_info, match, input_info_re)) {
                    const std::string type = match[1];
                    const std::string name = match[2];
                    const std::string def = match[3];

                    const std::string internal_name = toInputName(name);
                    const std::string internal_name_shift = internal_name + "_shift";
                    const std::string internal_name_amp = internal_name + "_amp";
                    const std::string internal_name_res = internal_name + "_res";
                    const std::string internal_name_tc = internal_name + "_tc";

                    bool defined = params_.count(name) > 0;
                    std::optional<Address> addr_opt;
                    if (defined && isAddress(params_.at(name).value)) {
                        addr_opt = std::get<Address>(params_.at(name).value);
                    }

                    bool is_texture = addr_opt.has_value() && store->isMedia(addr_opt.value());

                    if (is_texture) {
                        frag_shader << "uniform sampler2D " << internal_name << ";\n";
                        frag_shader << "in vec2 " << internal_name_tc << ";\n";
                    } else {
                        frag_shader << "uniform " << type << " " << internal_name;

                        if (!defined && def != "") {
                            frag_shader << " = " << def;
                        }

                        frag_shader << ";\n";
                    }

                    frag_shader << "uniform float " <<  internal_name_shift << " = 0;\n";
                    frag_shader << "uniform float " <<  internal_name_amp << " = 1;\n";
                    frag_shader << "uniform vec2 " << internal_name_res << ";\n";

                    frag_shader << "#define " << name << "_res " << internal_name_res << "\n";
                    frag_shader << "#define " << name << "_tc ";
                    if (is_texture && addr_opt.has_value()) {
                        frag_shader << internal_name_tc;
                        uniforms_[internal_name_res] = addr_opt.value() + "resolution";
                    } else {
                        frag_shader << "vec2(0)";
                    }
                    frag_shader << "\n";

                    frag_shader << type << " input_" << name << "(in vec2 uv) {\n";
                    if (is_texture && addr_opt.has_value()) {
                        frag_shader << "   return " << "texture(" << internal_name << ", uv)";
                        std::string swiz = addr_opt.value().getSwiz();

                        // Expand the swizzle
                        if (swiz.empty()) {
                            swiz = "xyzw";
                        }

                        while (swiz.length() < 4) {
                            swiz += swiz.back();
                        }

                        if (type == "float") {
                            frag_shader << "." << swiz[0];
                        } else if (type == "vec2") {
                            frag_shader << "." << swiz[0] << swiz[1];
                        } else if (type == "vec3") {
                            frag_shader << "." << swiz[0] << swiz[1] << swiz[2];
                        } else if (type == "vec4") {
                            frag_shader << "." << swiz[0] << swiz[1] << swiz[2] << swiz[3];
                        } else if (type == "bool") {
                            frag_shader << "." << swiz[0] << " > 0.5";
                        } else {
                            throw std::runtime_error("unsupported input type '" + type + "'");
                        }
                    } else {
                        frag_shader << "  return " << internal_name;
                    }

                    if (type != "bool") {
                        frag_shader << " * " << internal_name_amp << " + " << internal_name_shift;
                    }

                    frag_shader << ";\n";
                    frag_shader << "}\n";

                    frag_shader << type << " input_" << name << "() { return input_" <<
                        name << "(" << name << "_tc); }\n";
                } else {
                    throw std::runtime_error("Unable to parse input definition line: " + line);
                }
            } else {
                frag_shader << line << "\n";
                continue;
            }

            frag_shader << "#line " << line_no + 1 << "\n";
        }

        program_->loadShaderStr(GL_VERTEX_SHADER, vert_shader, "internal-vert.glsl");
        program_->loadShaderStr(GL_FRAGMENT_SHADER, frag_shader.str(), path_);
        program_->compile();
    }

    void Module::setValues(std::shared_ptr<ValueStore> store, bool first_pass) {
        unsigned int slot = 0;

        for (const auto& kv : program_->getUniformTypes()) {
            const std::string& uni_name = kv.first;

            // Do we have info about this uniform?
            if (uniforms_.count(uni_name) == 0) {
                continue;
            }

            AddressOrValue& addr_or_val = uniforms_.at(uni_name);

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
