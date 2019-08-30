#ifndef FRAG_CMD_OVERWRITE_VAR_H_
#define FRAG_CMD_OVERWRITE_VAR_H_

// Ours
#include "../AddressOrValue.h"
#include "Command.h"

namespace vidrevolt {
    namespace cmd {
        struct OverwriteVar : public Command {
            Address target;
            AddressOrValue replacement;
        };
    }
}

#endif
