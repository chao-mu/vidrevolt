#ifndef VIDREVOLT_CONTROLLER_H_
#define VIDREVOLT_CONTROLLER_H_

// STL
#include <vector>
#include <string>
#include <functional>

// Ours
#include "Value.h"

namespace vidrevolt {
    class Controller {
        public:
            virtual ~Controller() = default;

            virtual void connect(const std::string& control_name, std::function<void(Value)> f) = 0;
            virtual std::vector<std::string> getControlNames() const = 0;

            virtual void tick();
    };
}

#endif
