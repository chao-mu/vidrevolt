// STL
#include <variant>

// Ours
#include "Video.h"
#include "Image.h"
#include "Value.h"
#include "Group.h"
#include "Controller.h"
#include "RenderStep.h"

namespace vidrevolt {
    using Referable = std::variant<Media*, Value, Group*, Controller*, RenderStep::Label>;
}
