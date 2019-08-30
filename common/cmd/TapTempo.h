#ifndef VIDREVOLT_CMD_TAPTEMPO_H_
#define VIDREVOLT_CMD_TAPTEMPO_H_

// Ours
#include "../Address.h"
#include "Command.h"

namespace vidrevolt {
    namespace cmd {
        struct TapTempo : public Command {
            Address target_bpm_sync;
        };
    }
}

#endif
