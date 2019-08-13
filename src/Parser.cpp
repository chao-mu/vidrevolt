#include "Parser.h"

// STL
#include <regex>

namespace frag {
    Parser::Parser() : store_(std::make_shared<ValueStore>()) {}

    AddressOrValue Parser::readAddressOrValue(const YAML::Node& node) {
        if (node.IsSequence()) {
            std::vector<float> v = {};

            for (const auto& el : node) {
                v.push_back(el.as<float>());
            }

            return Value(v);
        }

        const std::string str = node.as<std::string>();

        bool b;
        float f;
        if (YAML::convert<bool>::decode(node, b) && str != "n" && str != "y") {
            return Value(b);
        }

        if (YAML::convert<float>::decode(node, f)) {
            return Value(f);
        }

        return readAddress(node);
    }

    Address Parser::readAddress(const YAML::Node& node) {
        std::string str = node.as<std::string>();
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream iss(str);
        while (std::getline(iss, token, '.')) {
            tokens.push_back(token);
        }

        std::string swiz;
        if (tokens.size() > 1) {
            std::regex nonswiz_re("[^xyzwrgb]");
            if (!std::regex_search(tokens.back(), nonswiz_re)) {
                Address no_swiz_addr(tokens);

                if (store_->getGroup(no_swiz_addr.withoutTail()) == nullptr) {
                    swiz = tokens.back();
                    tokens.pop_back();
                }
            }
        }

        Address addr(tokens);
        addr.setSwiz(swiz);

        return addr;
    }

    const YAML::Node Parser::requireNode(const YAML::Node& parent, const std::string& key, const std::string& err) {
        if (!parent[key]) {
            throw std::runtime_error(err);
        }

        return parent[key];
    }


    Address Parser::requireAddress(const YAML::Node& parent, const std::string& key, const std::string& err) {
        return readAddress(requireNode(parent, key, err));
    }

    Trigger Parser::readTrigger(const YAML::Node& node) {
        Trigger trig;
        if (node.IsSequence()) {
            for (const auto& addr_node : node) {
                trig.push_back(readAddress(addr_node));
            }
        } else {
            trig.push_back(readAddress(node));
        }

        return trig;
    }

    Trigger Parser::requireTrigger(
            const YAML::Node& parent,
            const std::string& key,
            const std::string& err) {
        if (!parent[key]) {
            throw std::runtime_error(err);
        }

        return readTrigger(parent[key]);
    }
}
