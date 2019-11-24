#ifndef VIDREVOLT_GL_MODULE_H_
#define VIDREVOLT_GL_MODULE_H_

// STL
#include <string>
#include <memory>

// Ours
#include "gl/ShaderProgram.h"
#include "AddressOrValue.h"
#include "RenderStep.h"
#include "gl/ParamSet.h"

namespace vidrevolt {
    namespace gl {
        class Module {
            public:
                using UniformNeeds = std::map<std::string, AddressOrValue>;

                struct Input {
                    std::string name;
                    std::string type;
                    std::string default_value;
                };

                void compile(const std::string& path);

                std::shared_ptr<ShaderProgram> getShaderProgram();

                const std::vector<Input>& getInputs();

                UniformNeeds getNeeds(ParamSet params);

            private:
                std::pair<std::string , std::vector<Module::Input>> readFragShader(const std::string& path);

                std::shared_ptr<ShaderProgram> program_;
                std::vector<Input> inputs_;
        };
    }
}
#endif
