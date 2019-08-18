#include "OverwriteGroup.h"

// STL
#include <stdexcept>

namespace frag {
    namespace cmd {
        void OverwriteGroup::run(std::shared_ptr<ValueStore> store) const {
            store->setGroupMember(
                std::get<Address>(args_.at(0)),
                args_.at(1)
            );
        }

        void OverwriteGroup::validate() const {
            if (args_.size() != 2) {
                throw std::runtime_error("Command '" + name_ + "' requires 2 arguments, arg 1: src, arg 2: dest.");
            }

            if (!isAddress(args_.at(0))) {
                throw std::runtime_error(
                        "Command '" + name_ + "' expected argument 1 to be an address");
            }
        }
    }
}
