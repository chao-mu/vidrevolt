#include "RenderStep.h"

// STL
#include <sstream>
#include <regex>

// Ours
#include "fileutil.h"

namespace vidrevolt {
    RenderStep::RenderStep(
            const std::string& output,
            const std::string& path,
            const Resolution& res
        ) : output_(output), path_(path), resolution_(res) {}

    Resolution RenderStep::getResolution() {
        return resolution_;
    }

    std::map<std::string, RenderStep::Param> RenderStep::getParams() {
        return params_;
    }

    void RenderStep::setParam(const std::string& name, Param p) {
        params_[name] = p;
    }

    std::string RenderStep::getOutput() const {
        return output_;
    }

    std::string RenderStep::getPath() const {
        return path_;
    }
}
