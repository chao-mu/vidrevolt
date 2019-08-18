#ifndef FRAG_PARSER_H_
#define FRAG_PARSER_H_

// yaml-cpp
#include "yaml-cpp/yaml.h"

// Ours
#include "Address.h"
#include "ValueStore.h"

namespace vidrevolt {
    class Parser {
        public:
            Parser();

        protected:
            AddressOrValue readAddressOrValue(const YAML::Node& node, bool parse_swiz);
            Address readAddress(const YAML::Node& node, bool parse_swiz);
            Trigger readTrigger(const YAML::Node& node);
            const YAML::Node requireNode(const YAML::Node& parent, const std::string& key, const std::string& err);
            Address requireAddress(const YAML::Node& parent, const std::string& key, const std::string& err, bool parse_swiz);
            Trigger requireTrigger(const YAML::Node& parent, const std::string& key, const std::string& err);

            std::shared_ptr<ValueStore> store_;
    };
}

#endif
