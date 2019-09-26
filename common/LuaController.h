#ifndef VIDREVOLT_LUACONTROLLER_H_
#define VIDREVOLT_LUACONTROLLER_H_

// STL
#include <string>
#include <map>
#include <vector>

// Lua
#include "lua5.3/lua.hpp"

// Ours
#include "Controller.h"
#include "AddressOrValue.h"

namespace vidrevolt {
    class LuaController : public Controller {
        public:
            struct Control {
                std::string control_name;
                std::string func_name;
                std::vector<AddressOrValue> params;
            };

            LuaController(const std::string& path, bool shared);
            ~LuaController();

            virtual void beforePoll() override;

            const std::map<std::string, LuaController::Control>& getControls() const;
            void setControlArgs(const std::string& func_name, std::vector<vidrevolt::Value> args);

            void addControl(const LuaController::Control& ctrl);

        private:
            lua_State* getLua(const std::string& control_name);

            bool shared_ = false;
            std::map<std::string, Control> controls_;
            std::map<std::string, std::vector<vidrevolt::Value>> control_args_;
            std::map<std::string, lua_State*> control_states_;
            lua_State* L_ = nullptr;
            std::string path_;
    };
}

#endif
