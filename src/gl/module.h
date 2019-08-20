#ifndef VIDREVOLT_GL_MODULE_H_
#define VIDREVOLT_GL_MODULE_H_

// STL
#include <string>
#include <map>

// Ours
#include "ShaderProgram.h"
#include "../AddressOrValue.h"
#include "../ValueStore.h"
#include "../Module.h"

namespace vidrevolt {
    namespace gl {
        namespace module {
            using UniformNeeds = std::map<std::string, AddressOrValue>;

            std::string toInternalName(const std::string& name);

            std::pair<std::shared_ptr<ShaderProgram>, UniformNeeds> compile(
                   const std::string path,
                   std::map<std::string, Module::Param> params,
                   std::shared_ptr<ValueStore> store);

            std::pair<std::string, UniformNeeds> readVertShader(const std::map<std::string, Module::Param>& params, std::shared_ptr<ValueStore> store);

            std::pair<std::string, UniformNeeds> readFragShader(const std::string path, std::map<std::string, Module::Param> params, std::shared_ptr<ValueStore> store);
        }
    }
}

#endif
