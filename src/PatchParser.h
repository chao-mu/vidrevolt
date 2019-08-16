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
#include "Texture.h"
#include "Module.h"
#include "Resolution.h"
#include "AddressOrValue.h"
#include "Trigger.h"
#include "midi/Device.h"
#include "cmd/Command.h"
#include "Parser.h"

namespace frag {
    class PatchParser : Parser {
        public:
            PatchParser(const std::string& path);

            void parse();

            std::map<std::string, std::shared_ptr<Video>> getVideos();
            std::map<std::string, std::shared_ptr<Texture>> getImages();
            std::vector<std::shared_ptr<Module>> getModules();
            std::map<std::string, std::shared_ptr<midi::Device>> getControllers();
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

            std::shared_ptr<Texture> loadImage(const std::string& name, const std::string& path, const YAML::Node& settings) const;
            std::shared_ptr<Video> loadVideo(const std::string& name, const std::string& path, const YAML::Node& settings) const;
            std::shared_ptr<midi::Device> loadMidiDevice(const std::string& name, const YAML::Node& settings) const;
            std::shared_ptr<cmd::Command> loadCommand(int num, const std::string& name, Trigger trigger, std::vector<AddressOrValue> args);

            const std::string path_;
            std::map<std::string, std::shared_ptr<Video>> videos_;
            std::map<std::string, std::shared_ptr<Texture>> images_;
            std::map<std::string, std::shared_ptr<midi::Device>> controllers_;
            std::vector<std::shared_ptr<Module>> modules_;
            std::map<std::string, std::shared_ptr<Group>> groups_;
            std::vector<std::shared_ptr<cmd::Command>> commands_;
    };
}

#endif
