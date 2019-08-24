#ifndef VIDREVOLT_KEYBOARD_H_
#define VIDREVOLT_KEYBOARD_H_

// STL
#include <map>
#include <memory>

// Boost
#include <boost/signals2.hpp>

// Ours
#include "gl/GLUtil.h"
#include "Value.h"
#include "Controller.h"

namespace vidrevolt {
    class Keyboard : public Controller {
        public:
            Keyboard();

            virtual void connect(const std::string& control_name, std::function<void(Value)> f) override;
            virtual std::vector<std::string> getControlNames() const override;

            static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods);

        private:
            static std::map<int, boost::signals2::signal<void(Value)>> signals_;

            std::map<std::string, int> codes_;
            std::vector<std::string> control_names_;
    };
}

#endif

