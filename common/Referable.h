// STL
#include <variant>

// Ours
#include "Video.h"
#include "Image.h"
#include "Value.h"
#include "Group.h"
#include "Controller.h"

namespace vidrevolt {
    using ModuleOutputLabel = std::string;
    using Referable = std::variant<Media*, Value, Group*, Controller*, ModuleOutputLabel>;
}
