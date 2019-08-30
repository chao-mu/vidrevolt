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
        ) : output_(output), path_(path), resolution_(res) {}

    Resolution Module::getResolution() {
        return resolution_;
    }

    std::map<std::string, Module::Param> Module::getParams() {
        return params_;
    }

    void Module::setParam(const std::string& name, Param p) {
        params_[name] = p;
    }

    std::string Module::getOutput() const {
        return output_;
    }

    std::string Module::getPath() const {
        return path_;
    }
}
