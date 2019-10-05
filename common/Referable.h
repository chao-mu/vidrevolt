// STL
#include <variant>

// Ours
#include "Video.h"
#include "Image.h"
#include "Value.h"
#include "Controller.h"
#include "RenderStep.h"

namespace vidrevolt {
    using Referable = std::variant<Media*, Value, Controller*, RenderStep::Label>;
}
