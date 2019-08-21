#include "Parser.h"

// STL
#include <regex>

namespace vidrevolt {
    Parser::Parser() : store_(std::make_shared<ValueStore>()) {}

    Address Parser::resolveAddress(const Address& addr) {
        return store_->getAddressDeep(addr);
    }

    AddressOrValue Parser::readAddressOrValue(const YAML::Node& node, bool parse_swiz) {
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

        return readAddress(node, parse_swiz);
    }

    Address Parser::readAddress(const YAML::Node& node, bool parse_swiz) {
        std::string str = node.as<std::string>();
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream iss(str);
        while (std::getline(iss, token, '.')) {
            tokens.push_back(token);
        }

        std::string swiz;
        if (parse_swiz) {
            if (tokens.size() > 1) {
                std::regex nonswiz_re("[^xyzwrgb]");
                if (!std::regex_search(tokens.back(), nonswiz_re)) {
                    Address no_swiz_addr = Address(tokens).withoutBack();

                    if (store_->getMedia(no_swiz_addr) != nullptr) {
                        swiz = tokens.back();
                        tokens.pop_back();
                    }
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


    Address Parser::requireAddress(const YAML::Node& parent, const std::string& key, const std::string& err, bool parse_swiz) {
        return readAddress(requireNode(parent, key, err), parse_swiz);
    }

    Trigger Parser::readTrigger(const YAML::Node& node) {
        /*Trigger trig;
        if (node.IsSequence()) {
            for (const auto& addr_node : node) {
                trig.push_back(readAddress(addr_node, false));
            }
        } else {
            trig.push_back(readAddress(node, false));
        }

        return trig;
        */
        return readAddress(node, false);
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
