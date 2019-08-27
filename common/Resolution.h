#ifndef FRAG_RESOLUTION_H_
#define FRAG_RESOLUTION_H_

#include <string>

namespace vidrevolt {
    struct Resolution {
        int width = 0;
        int height = 0;

        std::string str() const;
    };
}

#endif
