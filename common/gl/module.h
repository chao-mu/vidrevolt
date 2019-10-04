#ifndef VIDREVOLT_GL_MODULE_H_
#define VIDREVOLT_GL_MODULE_H_

// STL
#include <string>
#include <map>

// Ours
#include "ShaderProgram.h"
#include "../AddressOrValue.h"
#include "../Patch.h"
#include "../RenderStep.h"

namespace vidrevolt {
    namespace gl {
        namespace module {
            using UniformNeeds = std::map<std::string, AddressOrValue>;

            std::string toInternalName(const std::string& input_name, const std::string& field);

            std::shared_ptr<ShaderProgram> compile(
                   const std::string path,
                   std::map<std::string, RenderStep::Param> params,
                   std::shared_ptr<Patch> patch);

            std::string readVertShader(
                    const std::map<std::string, RenderStep::Param>& params,
                    std::shared_ptr<Patch> patch);

            std::string readFragShader(
                    const std::string path,
                    std::map<std::string, RenderStep::Param> params,
                    std::shared_ptr<Patch> patch);

            UniformNeeds getNeeds(std::map<std::string, RenderStep::Param> params);
        }
    }
}

#endif
