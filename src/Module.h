#ifndef VIDREVOLT_MODULE_H_
#define VIDREVOLT_MODULE_H_

// STL
#include <string>

// Ours
#include "AddressOrValue.h"
#include "Address.h"
#include "Value.h"
#include "ValueStore.h"

namespace vidrevolt {
    class Module {
        public:
            struct Param {
                public:
                    AddressOrValue value;
                    AddressOrValue amp;
                    AddressOrValue shift;
                    AddressOrValue pow;
            };

            Module(const std::string& output, const std::string& path, const Resolution& res);

            std::map<std::string, Module::Param> getParams();

            void setParam(const std::string& input, Param src);

            const std::string& getOutput() const;

            const std::string& getPath() const;

            Resolution getResolution();

        private:
            const std::string output_;
            const std::string path_;
            std::map<std::string, Param> params_;
            const Resolution resolution_;
    };
}

#endif
