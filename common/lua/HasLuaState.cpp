#include "HasLuaState.h"

// Ours
#include "../fileutil.h"

namespace vidrevolt {
    namespace lua {
        HasLuaState::HasLuaState(const std::string& path, bool shared) : path_(path), shared_(shared)  {}

        HasLuaState::~HasLuaState() {
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

        void HasLuaState::pushFunction(lua_State* L, const std::string& func_name) {
            lua_getglobal(L, func_name.c_str());
            if (!lua_isfunction(L, -1)) {
                throw std::runtime_error("Lua function not defined in " + getPath() + " : " + func_name);
            }
        }

        lua_State* HasLuaState::getLua(const std::string& key) {
            lua_State* L = nullptr;
            if (shared_) {
                L = L_;
            } else {
                L = control_states_[key];
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
                control_states_[key] = L;
            }

            return L;
        }

        std::string HasLuaState::getPath() {
            return path_;
        }
    }
}
