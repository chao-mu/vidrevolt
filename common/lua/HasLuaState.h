#ifndef VIDREVOLT_LUA_HASLUASTATE_H_
#define VIDREVOLT_LUA_HASLUASTATE_H_

// STL
#include <string>
#include <map>

// Lua
#include "lua5.3/lua.hpp"

namespace vidrevolt {
    namespace lua {
        class HasLuaState {
            public:
                virtual ~HasLuaState();

            protected:
                HasLuaState(const std::string& path, bool shared);

                lua_State* getLua(const std::string& key);
                std::string getPath();
                void pushFunction(lua_State* L, const std::string& func_name);

            private:
                std::map<std::string, lua_State*> control_states_;
                lua_State* L_ = nullptr;
                std::string path_;
                bool shared_ = false;
        };
    }
}
#endif
