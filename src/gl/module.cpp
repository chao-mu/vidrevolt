#include "module.h"

// STL
#include <regex>

// Ours
#include "../AddressOrValue.h"
#include "../fileutil.h"

namespace vidrevolt {
    namespace gl {
        namespace module {
            std::string toInternalName(const std::string& name) {
                return "vr_input_" + name;
            }

            std::pair<std::shared_ptr<ShaderProgram>, UniformNeeds> compile(
                   const std::string path,
                   std::map<std::string, Module::Param> params,
                   std::shared_ptr<ValueStore> store) {

                UniformNeeds uniforms;

                auto program = std::make_shared<gl::ShaderProgram>();

                std::pair<std::string, gl::module::UniformNeeds> vert_shader = readVertShader(params, store);
                std::pair<std::string, gl::module::UniformNeeds> frag_shader = readFragShader(path, params, store);

                uniforms.insert(vert_shader.second.cbegin(), vert_shader.second.cend());
                uniforms.insert(frag_shader.second.cbegin(), frag_shader.second.cend());

                program->loadShaderStr(GL_VERTEX_SHADER, vert_shader.first, "internal-vert.glsl");
                program->loadShaderStr(GL_FRAGMENT_SHADER, frag_shader.first, path);

                program->compile();

                return std::make_pair(program, uniforms);
            }


            std::pair<std::string, UniformNeeds> readVertShader(const std::map<std::string, Module::Param>& params, std::shared_ptr<ValueStore> store) {
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

                    uniform vec2 iResolution;

                    layout (location = 0) in vec3 aPos;
                )V";

                for (const auto& kv : params) {
                    if (!isAddress(kv.second.value) ||
                            !store->isMedia(std::get<Address>(kv.second.value))) {
                        continue;
                    }

                    const std::string& name = kv.first;
                    const std::string internal_name = toInternalName(name);

                    shader += "uniform vec2 " + internal_name + "_res;\n";
                    shader += "out vec2 " + internal_name + "_tc;\n";
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
                    if (!isAddress(kv.second.value) || !store->isMedia(std::get<Address>(kv.second.value))) {
                        continue;
                    }

                    const std::string& name = kv.first;
                    const std::string internal_name = toInternalName(name);
                    const std::string res_name = internal_name + "_res";
                    const std::string tc_name = internal_name + "_tc";

                    shader += tc_name + " = uv_in;\n";
                    shader += tc_name + ".x /= " + res_name + ".x / " + res_name + ".y;\n";
                    shader += tc_name + " += .5;\n";
                }

                shader += "}";

                return std::make_pair(shader, UniformNeeds());
            }

            std::pair<std::string, UniformNeeds> readFragShader(const std::string path, std::map<std::string, Module::Param> params, std::shared_ptr<ValueStore> store) {
                UniformNeeds uniforms;

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

                            const std::string internal_name = toInternalName(name);
                            const std::string internal_name_shift = internal_name + "_shift";
                            const std::string internal_name_amp = internal_name + "_amp";
                            const std::string internal_name_res = internal_name + "_res";
                            const std::string internal_name_tc = internal_name + "_tc";
                            const std::string internal_name_pow = internal_name + "_pow";

                            bool defined = params.count(name) > 0;
                            std::optional<Address> addr_opt;
                            if (defined) {
                                Module::Param& p = params.at(name);

                                uniforms[internal_name] = p.value;
                                uniforms[internal_name_amp] = p.amp;
                                uniforms[internal_name_shift] = p.shift;
                                uniforms[internal_name_pow] = p.pow;

                                if (isAddress(p.value)) {
                                    addr_opt = std::get<Address>(p.value);
                                }
                            }

                            bool is_texture = addr_opt.has_value() && store->isMedia(addr_opt.value());
                            if (is_texture) {
                                frag_shader << "uniform sampler2D " << internal_name << ";\n";
                                frag_shader << "in vec2 " << internal_name_tc << ";\n";
                            } else {
                                frag_shader << "uniform " << type << " " << internal_name;

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
                                uniforms[internal_name_res] = addr_opt.value() + "resolution";
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
                                frag_shader << "texture(" << internal_name << ", uv)";
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
                                frag_shader << internal_name;
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

                return std::make_pair(frag_shader.str(), uniforms);
            }
        }
    }
}
