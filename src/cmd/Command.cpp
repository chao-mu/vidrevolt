#include "Command.h"

// STL
#include <stdexcept>

namespace vidrevolt {
    namespace cmd {
        Command::Command(const std::string& name, const Trigger& trigger, std::vector<AddressOrValue> args) :
            name_(name), trigger_(trigger), args_(args) {}

        void Command::throwIncompatible() const {
            throw std::runtime_error(
                    "Command '" + name_ + "' is incompatible with target '" + target_.str() + "'");
        }

        std::string Command::getName() const {
            return name_;
        }

        Trigger Command::getTrigger() const {
            return trigger_;
        }
    }

}

