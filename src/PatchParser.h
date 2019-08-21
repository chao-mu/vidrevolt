#ifndef FRAG_PATCHPARSER_H_
#define FRAG_PATCHPARSER_H_

// STL
#include <map>
#include <optional>
#include <utility>
#include <variant>

// yaml-cpp
#include "yaml-cpp/yaml.h"

// Ours
#include "Video.h"
#include "Image.h"
#include "Module.h"
#include "Resolution.h"
#include "AddressOrValue.h"
#include "Trigger.h"
#include "midi/Device.h"
#include "Controller.h"
#include "cmd/Command.h"
#include "Parser.h"
#include "BPMSync.h"

namespace vidrevolt {
    class PatchParser : Parser {
        public:
            PatchParser(const std::string& path);

            void parse();

            std::map<std::string, std::shared_ptr<Video>> getVideos();
            std::map<std::string, std::shared_ptr<Image>> getImages();
            std::vector<std::shared_ptr<Module>> getModules();
            std::map<std::string, std::shared_ptr<Controller>> getControllers();
            std::map<std::string, std::shared_ptr<Group>> getGroups();
            std::shared_ptr<ValueStore> getValueStore();
            std::vector<std::shared_ptr<cmd::Command>> getCommands();

            Resolution getResolution() const;

        private:
            void parseMedia(const YAML::Node& patch);
            void parseCommands(const YAML::Node& patch);
            void parseControllers(const YAML::Node& patch);
            void parseModules(const YAML::Node& patch);
            void parseGroups(const YAML::Node& patch);
            void parseVars(const YAML::Node& patch);

            std::string getBuiltinShader(const std::string& path);

            void connectCommand(std::shared_ptr<cmd::Command> c);

            std::shared_ptr<Image> loadImage(const std::string& name, const std::string& path, const YAML::Node& settings) const;
            std::shared_ptr<Video> loadVideo(const std::string& name, const std::string& path, const YAML::Node& settings) const;
            std::shared_ptr<midi::Device> loadMidiDevice(const std::string& name, const YAML::Node& settings) const;
            std::shared_ptr<cmd::Command> loadCommand(int num, const std::string& name, Trigger trigger, std::vector<AddressOrValue> args);

            const std::string path_;
            std::map<std::string, std::shared_ptr<Video>> videos_;
            std::map<std::string, std::shared_ptr<Image>> images_;
            std::map<std::string, std::shared_ptr<Controller>> controllers_;
            std::vector<std::shared_ptr<Module>> modules_;
            std::map<std::string, std::shared_ptr<Group>> groups_;
            std::map<Address, std::shared_ptr<BPMSync>> bpm_syncs_;
            std::vector<std::shared_ptr<cmd::Command>> commands_;
    };
}

#endif
