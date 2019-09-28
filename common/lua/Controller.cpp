#include "Controller.h"

// STL
#include <iostream>

// Ours
#include "../fileutil.h"

namespace vidrevolt {
    namespace lua {
        Controller::Controller(const std::string& path, bool shared) : shared_(shared), path_(path) {}

        Controller::~Controller() {
            if (shared_) {
                if (L_ != nullptr) {
                    lua_close(L_);
                }
            } else {
                for (const auto& kv : control_states_) {
                    if (kv.second != nullptr) {
                        lua_close(kv.second);
                    }
                }
            }
        }

        lua_State* Controller::getLua(const std::string& control_name) {
            lua_State* L = nullptr;
            if (shared_) {
                L = L_;
            } else {
                L = control_states_[control_name];
            }

            if (L == nullptr) {
                L = luaL_newstate();
            } else {
                return L;
            }

            // Execute lua code
            std::string code = vidrevolt::fileutil::slurp(path_);
            int ret = luaL_dostring(L, code.c_str());
            if (ret != LUA_OK) {
                throw std::runtime_error("Failed to evaluate " + path_ + ": " + lua_tostring(L, -1));
            }

            if (shared_) {
                L_ = L;
            } else {
                control_states_[control_name] = L;
            }

            return L;
        }

        void Controller::addControl(const Controller::Control& ctrl) {
            lua_State* L = getLua(ctrl.control_name);

            std::string func_name = ctrl.func_name;
            lua_getglobal(L, func_name.c_str());
            if (!lua_isfunction(L, -1)) {
                throw std::runtime_error("Lua function not defined in " + path_ + " : " + func_name);
            }

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
                lua_getglobal(L, func.c_str());
                if (!lua_isfunction(L, -1)) {
                    throw std::runtime_error("Lua function not defined in " + path_ + " : " + func);
                }

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
