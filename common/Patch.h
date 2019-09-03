#ifndef VIDREVOLT_PATCH_H_
#define VIDREVOLT_PATCH_H_

// STL
#include <memory>

// Ours
#include "Video.h"
#include "Image.h"
#include "Module.h"
#include "Group.h"
#include "Controller.h"
#include "Keyboard.h"
#include "Referable.h"
#include "Trigger.h"
#include "cmd/Command.h"

namespace vidrevolt {
    class Patch {
        public:
            const std::map<std::string, std::unique_ptr<Video>>& getVideos() const;
            const std::map<std::string, std::unique_ptr<Image>>& getImages() const;
            const std::vector<std::unique_ptr<Module>>& getModules() const;
            const std::map<std::string, std::shared_ptr<Controller>>& getControllers() const;
            const std::map<std::string, std::unique_ptr<Group>>& getGroups() const;

            void setVideo(const std::string& key, std::unique_ptr<Video> vid);
            void setImage(const std::string& key, std::unique_ptr<Image> image);
            void setController(const std::string& key, std::shared_ptr<Controller> controller);
            void setGroup(const std::string& key, std::unique_ptr<Group> groups);
            void setResolution(const Resolution& res);
            void setAOV(const std::string& key, const AddressOrValue& aov);

            Resolution getResolution();

            void addModule(std::unique_ptr<Module> mod);

            bool isGroup(const Address& addr) const;
            bool isMedia(const Address& addr) const;

            void visitReferable(const Address& addr, std::function<void(const std::string&, Referable)> f, const Address& tail=Address()) const;
            void visitGroupMember(const Address& addr, std::function<void(Group&, const std::string&)> f) const;

            void startRender();
            void endRender();

            void interpretCommand(cmd::Command* cmd);

            void addCommand(const Trigger& t, std::unique_ptr<cmd::Command> c);

        private:
            std::string getAddressDeep(const Address& addr);

            std::map<std::string, std::unique_ptr<Video>> videos_;
            std::map<std::string, std::unique_ptr<Image>> images_;
            std::map<std::string, std::shared_ptr<Controller>> controllers_;
            std::vector<std::unique_ptr<Module>> modules_;
            std::map<std::string, std::unique_ptr<Group>> groups_;
            std::map<std::string, AddressOrValue> aovs_;
            std::map<std::string, Resolution> module_resolutions_;
            std::vector<std::unique_ptr<cmd::Command>> commands_;
            Resolution resolution_;
    };
}

#endif

