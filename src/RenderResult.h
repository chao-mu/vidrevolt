#ifndef VIDREVOLT_RENDERRESULT_H_
#define VIDREVOLT_RENDERRESULT_H_

// STL
#include <memory>

// Ours
#include "gl/RenderOut.h"

namespace vidrevolt {
    struct RenderResult {
        std::shared_ptr<gl::RenderOut> primary;
        std::shared_ptr<gl::RenderOut> aux;
    };
}

#endif
