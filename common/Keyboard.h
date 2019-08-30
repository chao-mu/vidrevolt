#ifndef VIDREVOLT_KEYBOARD_H_
#define VIDREVOLT_KEYBOARD_H_

// STL
#include <map>
#include <memory>
#include <mutex>

// Boost
#include <boost/signals2.hpp>

// Ours
#include "Value.h"
#include "Controller.h"

namespace vidrevolt {
    class Keyboard : public Controller {
        public:
            Keyboard();
            void onKey(const std::string& name, bool press);

        private:
            virtual std::vector<std::string> getControlNames() const override;

            std::vector<std::string> control_names_;
    };
}

#endif

