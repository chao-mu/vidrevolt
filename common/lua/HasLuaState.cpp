#include "HasLuaState.h"

// Ours
#include "../fileutil.h"

namespace vidrevolt {
    namespace lua {
        HasLuaState::HasLuaState(const std::string& path, bool shared) : path_(path), shared_(shared)  {}

        HasLuaState::~HasLuaState() {
            for (const auto& kv : lua_states_) {
                if (kv.second != nullptr) {
                    lua_close(kv.second);
                }
            }
        }

        void HasLuaState::pushFunction(lua_State* L, const std::string& func_name) {
            lua_getglobal(L, func_name.c_str());
            if (!lua_isfunction(L, -1)) {
                throw std::runtime_error(func_name + " not defined in " + getPath());
            }
        }

        std::string HasLuaState::getPath() {
            return path_;
        }

        lua_State* HasLuaState::getLua(const std::string& key_in) {
            lua_State* L = nullptr;
            std::string key = shared_ ? "" : key_in;

            // Return if already initialized
            L = lua_states_[key];
            if (L != nullptr) {
                return L;
            }

            L = luaL_newstate();
            *static_cast<HasLuaState**>(lua_getextraspace(L)) = this;

            // Execute lua code
            std::string code = vidrevolt::fileutil::slurp(path_);
            int ret = luaL_dostring(L, code.c_str());
            if (ret != LUA_OK) {
                throw std::runtime_error("Failed to evaluate " + path_ + ": " + lua_tostring(L, -1));
            }

            // Store
            lua_states_[key] = L;

            return L;
        }
    }
}
