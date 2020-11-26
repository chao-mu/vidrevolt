#ifndef VIDREVOLT_LUAFRONTEND_H_
#define VIDREVOLT_LUAFRONTEND_H_

// Ours
#include "Pipeline.h"

// Sol (Lua)
#include <sol/sol.hpp>

namespace vidrevolt {
    class LuaFrontend {
        public:
            using ObjID = std::string;

            LuaFrontend(const std::string& path, std::shared_ptr<Pipeline> pipeline);

            void load();

            std::shared_ptr<gl::RenderOut> render();

        private:
            AddressOrValue toAOV(sol::object obj);

            ObjID luafunc_Video(const std::string& path, const sol::table& args);
            ObjID luafunc_Image(const std::string& path);
            ObjID luafunc_Keyboard();
            ObjID luafunc_BPM();
            ObjID luafunc_OSC(const std::string& path, int port);
            ObjID luafunc_Midi(const std::string& path);
            sol::table luafunc_getControlValues(const ObjID& controller_id);
            std::string luafunc_rend(const std::string& target, const std::string& path, sol::table inputs);
            void luafunc_flipPlayback(const std::string& id);
            void luafunc_tap(const std::string& sync_id);
            void luafunc_setFPS(const std::string& id, double fps);

            ObjID connect(const std::string& controller_id);

            sol::state lua_;
            const std::string path_;
            std::shared_ptr<Pipeline> pipeline_;
            std::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> last_time_;
    };
}
#endif
