#ifndef FRAG_CMD_OVERWRITE_GROUP_H_
#define FRAG_CMD_OVERWRITE_GROUP_H_

// Ours
#include "../AddressOrValue.h"
#include "Command.h"

namespace vidrevolt {
    namespace cmd {
        struct OverwriteGroup : public Command {
            Address target;
            AddressOrValue replacement;
        };
    }
}

#endif
