#ifndef FRAG_CMD_OVERWRITE_VAR_H_
#define FRAG_CMD_OVERWRITE_VAR_H_

// Ours
#include "Command.h"
#include "../Video.h"

namespace vidrevolt {
    namespace cmd {
        class OverwriteVar : public Command {
            public:
                using Command::Command;

                virtual void run(std::shared_ptr<ValueStore> store) const override;
                virtual void validate() const override;
        };
    }
}

#endif
