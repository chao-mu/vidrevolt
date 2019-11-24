#include "Resolution.h"

namespace vidrevolt {
    std::string Resolution::str() const {
        return std::to_string(width) + "x" + std::to_string(height);
    }
}
