#ifndef VIDREVOLT_PATCH_H_
#define VIDREVOLT_PATCH_H_

// STL
#include <memory>

// Sol (Lua)
#include <sol/sol.hpp>

// Ours
#include "Video.h"
#include "Image.h"
#include "RenderStep.h"
#include "Controller.h"
#include "Keyboard.h"
#include "Trigger.h"
#include "BPMSync.h"
#include "gl/Renderer.h"

namespace vidrevolt {
    class Pipeline {
        public:
            using ObjID = std::string;

            Pipeline(const std::string& path);

            void load();

            void setVideo(const std::string& key, std::unique_ptr<Video> vid);
            void setBPMSync(const std::string& key, std::shared_ptr<BPMSync> vid);
            void setController(const std::string& key, std::shared_ptr<Controller> controller);

            Resolution getResolution();

            std::shared_ptr<gl::RenderOut> render();
            void reconnectControllers();

        private:
            AddressOrValue toAOV(sol::object obj);

            ObjID next_id(const std::string& comment);

            ObjID luafunc_Video(const std::string& path, sol::table args);
            ObjID luafunc_Image(const std::string& path);
            ObjID luafunc_Keyboard();
            ObjID luafunc_BPM();
            ObjID luafunc_OSC(const std::string& path, int port);
            ObjID luafunc_Midi(const std::string& path);
            sol::table luafunc_getControlValues(const ObjID& controller_id);
            void luafunc_rend(const std::string& target, const std::string& path, sol::table inputs);
            void luafunc_flipPlayback(const std::string& id);
            void luafunc_tap(const std::string& sync_id);
            void luafunc_preload(sol::table shaders);

            std::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> last_time_;

            std::map<std::string, std::shared_ptr<BPMSync>> bpm_syncs_;
            std::map<Address, std::unique_ptr<Video>> videos_;
            std::map<std::string, std::shared_ptr<Controller>> controllers_;
            Resolution resolution_;
            sol::state lua_;
            std::unique_ptr<gl::Renderer> renderer_ = std::make_unique<gl::Renderer>();

            std::map<Address, bool> in_use_;
            std::map<Address, bool> last_in_use_;

            const std::string path_;
            size_t obj_id_cursor_ = 0;
    };
}

#endif
