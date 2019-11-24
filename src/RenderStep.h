#ifndef VIDREVOLT_RENDERSTEP_H_
#define VIDREVOLT_RENDERSTEP_H_

// STL
#include <string>
#include <map>

// Ours
#include "AddressOrValue.h"
#include "Address.h"
#include "Value.h"
#include "Resolution.h"

namespace vidrevolt {
    class RenderStep {
        public:
            struct Param {
                AddressOrValue value;
                AddressOrValue amp;
                AddressOrValue shift;
                AddressOrValue pow;
            };

            using Label = std::string;

            RenderStep(const std::string& output, const std::string& path, const Resolution& res);

            std::map<std::string, RenderStep::Param> getParams();

            void setParam(const std::string& input, Param src);

            std::string getOutput() const;

            std::string getPath() const;

            Resolution getResolution();

        private:
            const std::string output_;
            const std::string path_;
            std::map<std::string, Param> params_;
            const Resolution resolution_;
    };
}

#endif
