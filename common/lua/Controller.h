#ifndef VIDREVOLT_LUA_CONTROLLER_H_
#define VIDREVOLT_LUA_CONTROLLER_H_

// STL
#include <string>
#include <map>
#include <vector>

// Lua
#include "lua5.3/lua.hpp"

// Ours
#include "../Controller.h"
#include "../AddressOrValue.h"

namespace vidrevolt {
    namespace lua {
        class Controller : public vidrevolt::Controller {
            public:
                struct Control {
                    std::string control_name;
                    std::string func_name;
                    std::vector<AddressOrValue> params;
                };

                Controller(const std::string& path, bool shared);
                ~Controller();

                virtual void beforePoll() override;

                const std::map<std::string, Controller::Control>& getControls() const;
                void setControlArgs(const std::string& func_name, std::vector<vidrevolt::Value> args);

                void addControl(const Controller::Control& ctrl);

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
}

#endif
