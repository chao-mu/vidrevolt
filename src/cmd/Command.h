#ifndef FRAG_CMD_COMMAND_H_
#define FRAG_CMD_COMMAND_H_

#include "../Group.h"
#include "../Video.h"
#include "../ValueStore.h"
#include "../Trigger.h"

namespace frag {
    namespace cmd {
        class Command {
            public:
                Command(const std::string& name, const Trigger& trigger, std::vector<AddressOrValue> args);

                virtual void run(std::shared_ptr<ValueStore> store) const = 0;

                virtual void validate() const = 0;

                Trigger getTrigger() const;
                std::string getName() const;

            protected:
                void throwIncompatible() const;

                const std::string name_;
                const Address target_;
                const Trigger trigger_;
                std::vector<AddressOrValue> args_;
        };
    }
}

#endif
