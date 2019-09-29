#ifndef VIDREVOLT_LUA_CONTROLLER_H_
#define VIDREVOLT_LUA_CONTROLLER_H_

// STL
#include <string>
#include <map>
#include <vector>

// Ours
#include "../Controller.h"
#include "../AddressOrValue.h"
#include "HasLuaState.h"

namespace vidrevolt {
    namespace lua {
        class Controller : public vidrevolt::Controller, public HasLuaState {
            public:
                Controller(const std::string& path, bool shared);

                struct Control {
                    std::string control_name;
                    std::string func_name;
                    std::vector<AddressOrValue> params;
                };

                virtual void beforePoll() override;

                const std::map<std::string, Controller::Control>& getControls() const;
                void setControlArgs(const std::string& func_name, std::vector<vidrevolt::Value> args);

                void addControl(const Controller::Control& ctrl);

            private:
                std::map<std::string, Control> controls_;
                std::map<std::string, std::vector<vidrevolt::Value>> control_args_;
        };
    }
}

#endif
