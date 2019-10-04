#include "Controller.h"

// STL
#include <iostream>

namespace vidrevolt {
    namespace lua {
        Controller::Controller(const std::string& path, bool shared) : HasLuaState(path, shared) {}

        void Controller::addControl(const Controller::Control& ctrl) {
            controls_[ctrl.control_name] = ctrl;
            addControlName(ctrl.control_name);
        }

        void Controller::setControlArgs(const std::string& func_name, std::vector<vidrevolt::Value> args) {
            control_args_[func_name] = args;
        }

        void Controller::beforePoll() {
            for (const auto& kv : control_args_) {
                const std::string& control_name = kv.first;
                const std::vector<Value>& values = kv.second;
                std::string func = controls_.at(control_name).func_name;

                lua_State* L = getLua(control_name);
                pushFunction(L, func);

                // Store each value of the
                int argc = 0;
                for (const auto& v : values) {
                    lua_newtable(L);
                    int i = 0;
                    for (float v_comp : v.getVec4()) {
                        lua_pushnumber(L, static_cast<double>(v_comp));
                        lua_rawseti(L, -2, i++);
                    }

                    argc++;
                }

                lua_pcall(L, argc, 1, 0);
                lua_Number x = lua_tonumber(L, -1);
                addValue(control_name, Value(static_cast<float>(x)));

                lua_settop(L, 0);
            }
        }

        const std::map<std::string, Controller::Control>& Controller::getControls() const {
            return controls_;
        }
    }
}
