#ifndef FRAG_CMD_REVERSE_H_
#define FRAG_CMD_REVERSE_H_

// Ours
#include "../Address.h"
#include "Command.h"

namespace vidrevolt {
    namespace cmd {
        struct Reverse : public Command {
            Address target;
        };
    }
}

#endif
