#ifndef VIDREVOLT_CMD_TAPTEMPO_H_
#define VIDREVOLT_CMD_TAPTEMPO_H_

// Ours
#include "../BPMSync.h"
#include "Command.h"

namespace vidrevolt {
    namespace cmd {
        class TapTempo : public Command {
            public:
                TapTempo(const std::string& name, const Trigger& trigger, std::vector<AddressOrValue> args, std::shared_ptr<BPMSync> sync);

                virtual void run(std::shared_ptr<ValueStore> store) const override;

            private:
                std::shared_ptr<BPMSync> sync_;
        };
    }
}

#endif
