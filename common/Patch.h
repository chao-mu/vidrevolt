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
#include "Group.h"
#include "Controller.h"
#include "Keyboard.h"
#include "Referable.h"
#include "Trigger.h"
#include "cmd/Command.h"
#include "lua/Controller.h"

namespace vidrevolt {
    class Patch {
        public:
            using ObjID = std::string;

            Patch(const std::string& path);

            void load();

            const std::map<std::string, std::unique_ptr<Video>>& getVideos() const;
            const std::map<std::string, std::unique_ptr<Image>>& getImages() const;
            const std::vector<std::unique_ptr<RenderStep>>& getRenderSteps() const;
            const std::map<std::string, std::shared_ptr<Controller>>& getControllers() const;
            const std::map<std::string, std::unique_ptr<Group>>& getGroups() const;

            void setVideo(const std::string& key, std::unique_ptr<Video> vid);
            void setImage(const std::string& key, std::unique_ptr<Image> image);
            void setController(const std::string& key, std::shared_ptr<Controller> controller);
            void setLuaController(const std::string& key, std::shared_ptr<lua::Controller> lua);
            void setGroup(const std::string& key, std::unique_ptr<Group> groups);
            void setResolution(const Resolution& res);
            void setAOV(const std::string& key, const AddressOrValue& aov);

            Resolution getResolution();

            void addRenderStep(std::unique_ptr<RenderStep> mod);

            bool isSwizzable(const Address& addr) const;
            bool isMedia(const Address& addr) const;

            void visitReferable(const Address& addr, std::function<void(const std::string&, Referable)> f, const Address& tail=Address()) const;
            void visitGroupMember(const Address& addr, std::function<void(Group&, const std::string&)> f) const;

            void startRender();
            void endRender();

            void interpretCommand(cmd::Command* cmd);

            void addCommand(const Trigger& t, std::unique_ptr<cmd::Command> c);

            bool hasRenderStep(const std::string& label);

        private:
            void populateRenderSteps();
            AddressOrValue toAOV(sol::object obj);

            ObjID luafunc_Video(const std::string& path, sol::table args);
            sol::table luafunc_getControlValues(const ObjID& controller_id);
            ObjID luafunc_Midi(const std::string& path);
            ObjID luafunc_rend(const std::string& label, const std::string& path, sol::table inputs);

            std::string getAddressDeep(const Address& addr);

            std::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> last_time_;

            std::map<std::string, std::unique_ptr<Video>> videos_;
            std::map<std::string, std::unique_ptr<Image>> images_;
            std::map<std::string, std::shared_ptr<Controller>> controllers_;
            std::map<std::string, std::shared_ptr<lua::Controller>> lua_controllers_;
            std::vector<std::unique_ptr<RenderStep>> render_steps_;
            std::map<std::string, std::unique_ptr<Group>> groups_;
            std::map<std::string, AddressOrValue> aovs_;
            std::map<std::string, Resolution> render_step_resolutions_;
            std::vector<std::unique_ptr<cmd::Command>> commands_;
            Resolution resolution_;
            sol::state lua_;

            const std::string path_;
    };
}

#endif
