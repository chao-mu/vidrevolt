#include "module.h"

// STL
#include <regex>

// Ours
#include "../AddressOrValue.h"
#include "../fileutil.h"

namespace vidrevolt {
    namespace gl {
        namespace module {
            std::string toInternalName(const std::string& input_name, const std::string& field) {
                return "vr_input_" + input_name + "_" + field;
            }

            UniformNeeds getNeeds(std::map<std::string, RenderStep::Param> params) {
                UniformNeeds uniforms;

                for (const auto& kv : params) {
                    const std::string& name = kv.first;
                    const RenderStep::Param& param = kv.second;

                    uniforms[toInternalName(name, "value")] = param.value;
                    uniforms[toInternalName(name, "shift")] = param.shift;
                    uniforms[toInternalName(name, "amp")] = param.amp;
                    uniforms[toInternalName(name, "pow")] = param.pow;

                    if (std::holds_alternative<Address>(param.value)) {
                        uniforms[toInternalName(name, "res")] =
                            std::get<Address>(param.value) + "resolution";
                    }
                }

                return uniforms;
            }

            std::shared_ptr<ShaderProgram> compile(
                   const std::string path,
                   std::map<std::string, RenderStep::Param> params,
                   std::shared_ptr<Patch> patch) {

                auto program = std::make_shared<gl::ShaderProgram>();

                std::string vert_shader = readVertShader(params, patch);
                std::string frag_shader = readFragShader(path, params, patch);

                program->loadShaderStr(GL_VERTEX_SHADER, vert_shader, "internal-vert.glsl");
                program->loadShaderStr(GL_FRAGMENT_SHADER, frag_shader, path);

                program->compile();

                return program;
            }


            std::string readVertShader(const std::map<std::string, RenderStep::Param>& params, std::shared_ptr<Patch> patch) {
                std::string shader = R"V(
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
                    out vec2 max_uv;

                    uniform vec2 iResolution;

                    layout (location = 0) in vec3 aPos;
                )V";

                for (const auto& kv : params) {
                    if (!isAddress(kv.second.value) ||
                            !patch->isMedia(std::get<Address>(kv.second.value))) {
                        continue;
                    }

                    const std::string& name = kv.first;

                    shader += "uniform vec2 " + toInternalName(name, "res") + ";\n";
                    shader += "out vec2 " + toInternalName(name, "tc") + ";\n";
                }

                shader += R"V(
                    void main() {
                        gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);

                        float widthStep = 1. / iResolution.x;
                        float heightStep = 1. / iResolution.y;

                        vec2 st = aPos.xy * .5 + .5;
                        vec2 uv_in = st - .5;
                        uv_in.x *= iResolution.x / iResolution.y;
                        uv = uv_in;
                        max_uv = vec2(iResolution.x / iResolution.y, 1);

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

                for (const auto& kv : params) {
                    if (!isAddress(kv.second.value) || !patch->isMedia(std::get<Address>(kv.second.value))) {
                        continue;
                    }

                    const std::string& name = kv.first;
                    const std::string res_name = toInternalName(name, "res");
                    const std::string tc_name = toInternalName(name, "tc");

                    shader += tc_name + " = uv_in;\n";
                    shader += tc_name + ".x /= " + res_name + ".x / " + res_name + ".y;\n";
                    shader += tc_name + " += .5;\n";
                }

                shader += "}";

                return shader;
            }

            std::string readFragShader(const std::string path, std::map<std::string, RenderStep::Param> params, std::shared_ptr<Patch> patch) {
                std::ifstream ifs(path);
                if (ifs.fail()) {
                    std::ostringstream err;
                    err << "Error loading " << path << " - " <<  std::strerror(errno);
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
                        frag_shader << fileutil::slurp(path, match[1]);
                    } else if (std::regex_match(line, match, pragma_input_re)) {
                        std::string input_info = match[1].str();
                        if (std::regex_match(input_info, match, input_info_re)) {
                            const std::string type = match[1];
                            const std::string name = match[2];
                            const std::string def = match[3];

                            const std::string internal_name_value = toInternalName(name, "value");
                            const std::string internal_name_shift = toInternalName(name, "shift");
                            const std::string internal_name_amp = toInternalName(name, "amp");
                            const std::string internal_name_res = toInternalName(name, "res");
                            const std::string internal_name_tc = toInternalName(name, "tc");
                            const std::string internal_name_pow = toInternalName(name, "pow");

                            bool defined = params.count(name) > 0;
                            std::optional<Address> addr_opt;
                            if (defined) {
                                RenderStep::Param& p = params.at(name);

                                if (isAddress(p.value)) {
                                    addr_opt = std::get<Address>(p.value);
                                }
                            }

                            bool is_texture = addr_opt.has_value() && patch->isMedia(addr_opt.value());
                            if (is_texture) {
                                frag_shader << "uniform sampler2D " << internal_name_value << ";\n";
                                frag_shader << "in vec2 " << internal_name_tc << ";\n";
                            } else {
                                frag_shader << "uniform " << type << " " << internal_name_value;

                                if (def != "") {
                                    frag_shader << " = " << def;
                                }

                                frag_shader << ";\n";
                            }

                            frag_shader << "uniform float " <<  internal_name_shift << " = 0;\n";
                            frag_shader << "uniform float " <<  internal_name_amp << " = 1;\n";
                            frag_shader << "uniform float " <<  internal_name_pow << " = 1;\n";
                            frag_shader << "uniform vec2 " << internal_name_res << " = vec2(0);\n";

                            frag_shader << "#define " << name << "_res " << internal_name_res << "\n";
                            frag_shader << "#define " << name << "_tc ";
                            if (is_texture && addr_opt.has_value()) {
                                frag_shader << internal_name_tc;
                            } else {
                                frag_shader << "vec2(0)";
                            }
                            frag_shader << "\n";

                            frag_shader << type << " input_" << name << "(in vec2 uv) {\n";
                            frag_shader << "   return ";

                            if (type != "bool") {
                                frag_shader << "pow(";
                            }

                            std::string pow_arg = "";
                            if (is_texture && addr_opt.has_value()) {
                                frag_shader << "texture(" << internal_name_value << ", uv)";
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
                                frag_shader << internal_name_value;
                            }

                            if (type != "bool") {
                                frag_shader << ", " <<  type << "(" << internal_name_pow << "))";
                                frag_shader << "* " << internal_name_amp << " + " << internal_name_shift;
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

                return frag_shader.str();
            }
        }
    }
}
