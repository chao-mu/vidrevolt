#ifndef VIDREVOLT_GL_MODULE_H_
#define VIDREVOLT_GL_MODULE_H_

// STL
#include <string>
#include <map>

// Ours
#include "ShaderProgram.h"
#include "../AddressOrValue.h"
#include "../RenderStep.h"

namespace vidrevolt {
    namespace gl {
        namespace module {
            using UniformNeeds = std::map<std::string, AddressOrValue>;

            struct Input {
                std::string name;
                std::string type;
                std::string default_value;
            };

            std::shared_ptr<ShaderProgram> compile(const std::string& path);

            UniformNeeds getNeeds(std::map<std::string, RenderStep::Param> params);
        }
    }
}

#endif
