#include "module.h"

// STL
#include <regex>

// Jinja2
#include <nlohmann/json.hpp>
#include <inja/inja.hpp>

// Ours
#include "../AddressOrValue.h"
#include "../fileutil.h"

namespace vidrevolt {
    namespace gl {
        namespace module {
            std::string toPrivateInputName(const std::string& input_name) {
                return "vr_input_" + input_name;
            }

            std::string inputStr(const Input& input) {
                nlohmann::json data;

                data["natural_name"] = input.name;
                data["private_name"] = toPrivateInputName(input.name);

                std::string type = input.type;
                data["type"] = type;

                size_t length = 0;
                std::string default_value;
                if (type == "float") {
                    length = 1;
                    default_value = "0.0";
                } else if (type == "bool") {
                    length = 1;
                    default_value = "false";
                } else if (type == "vec2") {
                    length = 2;
                    default_value = "vec2(0)";
                } else if (type == "vec3") {
                    length = 3;
                    default_value = "vec3(0)";
                } else if (type == "vec4") {
                    length = 4;
                    default_value = "vec4(0)";
                } else {
                    throw std::runtime_error("Unsupported input type '" + type + "'");
                }

                data["length"] = length;
                data["default"] = input.default_value == "" ? default_value : input.default_value;

                constexpr auto src = R"V(
                    uniform sampler2D {{private_name}}_as_tex;
                    uniform {{type}} {{private_name}}_as_value = {{default}};

                    {% for i in range(length) %}
                        uniform int {{private_name}}_swiz_{{i}} = {{i}};
                    {% endfor %}

                    uniform bool {{private_name}}_is_tex = false;

                    in vec2 {{private_name}}_tc;
                    #define {{natural_name}}_res {{private_name}}_res
                    #define {{natural_name}}_tc {{private_name}}_tc

                    {{type}} input_{{natural_name}}(in vec2 st) {
                        vec4 intermediate;
                        if ({{private_name}}_is_tex) {
                            vec4 intermediate = texture({{private_name}}_as_tex, st);
                            {{type}} ret;
                            {% if length > 1 %}
                                {% for i in range(length) %}
                                    ret[{{i}}] = intermediate[{{private_name}}_swiz_{{i}}];
                                {% endfor %}
                            {% else %}
                                float x = intermediate[{{private_name}}_swiz_0];
                                {% if type == "bool" %}
                                    ret = x > 0.5;
                                {% else %}
                                    ret = {{type}}(x);
                                {% end %}
                            {% endif %}

                            return ret;
                        } else {
                            {% if length > 1 %}
                                {{type}} ret;
                                {% for i in range(length) %}
                                    ret[{{i}}] = {{private_name}}_as_value[{{private_name}}_swiz_{{i}}];
                                {% endfor %}

                                return ret;
                            {% else %}
                                return {{private_name}}_as_value;
                            {% endif %}
                        }
                    }

                    {{type}} input_{{natural_name}}() {
                        return input_{{natural_name}}({{private_name}}_tc);
                    }
                )V";

                return inja::render(src, data);
            }

            std::string vertShaderStr(const std::vector<Input>& inputs) {
                nlohmann::json data;

                data["private_names"] = std::vector<std::string>{};
                for (const auto input : inputs) {
                    data["private_names"].push_back(toPrivateInputName(input.name));
                }

                constexpr auto src = R"V(
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

            {% for name in private_names %}
                    uniform vec2 {{name}}_res;
                    out vec2 {{name}}_tc;
                    uniform bool {{name}}_is_tex = false;
            {% endfor %}

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

            {% for name in private_names %}
                        if ({{name}}_is_tex) {
                            {{name}}_tc = uv_in;
                            {{name}}_tc.x /= {{name}}_res.x / {{name}}_res.y;
                            {{name}}_tc += .5;
                        } else {
                            {{name}}_tc = vec2(0);
                        }
            {% endfor %}
                    }
                )V";

                return inja::render(src, data);
            }

            UniformNeeds getNeeds(std::map<std::string, RenderStep::Param> params) {
                UniformNeeds uniforms;

                for (const auto& kv : params) {
                    const std::string& name = kv.first;
                    const RenderStep::Param& param = kv.second;

                    if (std::holds_alternative<Address>(param.value)) {
                        auto addr = std::get<Address>(param.value);

                        uniforms[toPrivateInputName(name) + "_as_tex"] = addr;
                        uniforms[toPrivateInputName(name) + "_is_tex"] = Value(true);
                        uniforms[toPrivateInputName(name) + "_res"] = addr + "resolution";
                    } else {
                        uniforms[toPrivateInputName(name) + "_as_value"] = param.value;
                    }
                }

                return uniforms;
            }

            std::pair<std::string , std::vector<Input>> readFragShader(const std::string& path) {
                std::ifstream ifs(path);
                if (ifs.fail()) {
                    std::ostringstream err;
                    err << "Error loading " << path << " - " <<  std::strerror(errno);
                    throw std::runtime_error(err.str());
                }

                std::vector<Input> inputs;

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

                            Input input = {name, type, def};
                            inputs.push_back(input);
                            frag_shader << inputStr(input);
                        } else {
                            throw std::runtime_error("Unable to parse input definition line: " + line);
                        }
                    } else {
                        frag_shader << line << "\n";
                        continue;
                    }

                    frag_shader << "#line " << line_no + 1 << "\n";
                }

                return {frag_shader.str(), inputs};
            }

            std::shared_ptr<ShaderProgram> compile(const std::string& path) {
                auto program = std::make_shared<gl::ShaderProgram>();

                auto [frag_shader, inputs] = readFragShader(path);
                std::string vert_shader = vertShaderStr(inputs);

                program->loadShaderStr(GL_VERTEX_SHADER, vert_shader, "internal-vert.glsl");
                program->loadShaderStr(GL_FRAGMENT_SHADER, frag_shader, path);

                program->compile();

                return program;
            }
        }
    }
}
