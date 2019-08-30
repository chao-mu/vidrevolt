#ifndef FRAG_CMD_Rotate_H_
#define FRAG_CMD_Rotate_H_

// Ours
#include "Address.h"
#include "Command.h"

namespace vidrevolt {
    namespace cmd {
        struct Rotate : public Command {
            Address target_group;
        };
    }
}

#endif
