#ifndef VIDREVOLT_PATCHBUILDER_H_
#define VIDREVOLT_PATCHBUILDER_H_

// STL
#include <memory>
#include <string>
#include <regex>

// yaml-cpp
#include "yaml-cpp/yaml.h"

// Ours
#include "Patch.h"
#include "Address.h"
#include "Trigger.h"

namespace vidrevolt {
    class PatchBuilder {
        public:
            void build(const std::string& path);
            std::shared_ptr<Patch> getPatch() const;

        private:
            void buildMedia(const YAML::Node& patch);
            void buildCommands(const YAML::Node& patch);
            void buildControllers(const YAML::Node& patch);
            void buildModules(const YAML::Node& patch);
            void buildGroups(const YAML::Node& patch);
            void buildVars(const YAML::Node& patch);
            void buildResolution(const YAML::Node& patch);

            AddressOrValue readAddressOrValue(const YAML::Node& node, bool parse_swiz);
            Address readAddress(const YAML::Node& node, bool parse_swiz);
            Trigger readTrigger(const YAML::Node& node);
            const YAML::Node requireNode(const YAML::Node& parent, const std::string& key, const std::string& err);
            Address requireAddress(const YAML::Node& parent, const std::string& key, const std::string& err, bool parse_swiz);
            Trigger requireTrigger(const YAML::Node& parent, const std::string& key, const std::string& err);

            void addImage(const std::string& name, const std::string& path, const YAML::Node& settings);
            void addVideo(const std::string& name, const std::string& path, const YAML::Node& settings);
            void addMidiDevice(const std::string& name, const YAML::Node& settings);
            void addOSCServer(const std::string& name, const YAML::Node& settings);
            void addCommand(int num, const std::string& name, Trigger trigger, std::vector<AddressOrValue> args);

            std::string getBuiltinShader(const std::string& path);

            std::shared_ptr<Patch> patch_ = std::make_shared<Patch>();
    };
}

#endif

