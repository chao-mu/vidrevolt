#ifndef VIDREVOLT_GL_PARAMSET_H_
#define VIDREVOLT_GL_PARAMSET_H_

// STL
#include <string>
#include <map>

// Ours
#include "AddressOrValue.h"

namespace vidrevolt {
    namespace gl {
        struct Param {
            AddressOrValue value;
            AddressOrValue amp = Value(1);
            AddressOrValue shift = Value(0);
            AddressOrValue pow = Value(1);
        };

        using ParamSet = std::map<std::string, Param>;
    }
}
#endif
