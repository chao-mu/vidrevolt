#include "OverwriteVar.h"

// STL
#include <stdexcept>

namespace frag {
    namespace cmd {
        void OverwriteVar::run(std::shared_ptr<ValueStore> store) const {
            Address dest = std::get<Address>(args_.at(0));

            if (store->getValue(dest).has_value()) {
                AddressOrValue aov = args_.at(1);
                if (!isValue(aov)) {
                    throw std::runtime_error(
                            "Command '" + name_ + "' expected argument 2 to be a value since it is overwriting a value");
                }

                store->set(dest, std::get<Value>(aov));
            } else {
                AddressOrValue aov = args_.at(1);
                if (!isAddress(aov)) {
                    throw std::runtime_error(
                            "Command '" + name_ + "' expected argument 2 to be an address since it is overwriting an address");
                }

                store->set(dest, std::get<Address>(aov));
            }
        }

        void OverwriteVar::validate() const {
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
