#include "Command.h"

// STL
#include <stdexcept>

namespace vidrevolt {
    namespace cmd {
        Command::Command(const std::string& name, const Trigger& trigger, std::vector<AddressOrValue> args) :
            name_(name), trigger_(trigger), args_(args) {}

        std::string Command::getName() const {
            return name_;
        }

        void Command::validate() const {}

        Trigger Command::getTrigger() const {
            return trigger_;
        }
    }

}

