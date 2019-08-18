#include "Group.h"

// STL
#include <algorithm>
#include <sstream>

namespace vidrevolt {
    void Group::add(AddressOrValue aov) {
        {
            std::lock_guard<std::mutex> guard(elements_mutex_);
            elements_.push_back(aov);
        }

        if (elements_.size() == 1)  {
            setMapping("first", 0);
        } else if (elements_.size() == 2)  {
            setMapping("second", 1);
        }
    }


    std::optional<AddressOrValue> Group::get(std::string mem) const {
         if (mappings_.count(mem) > 0) {
             std::lock_guard<std::mutex> guard(elements_mutex_);
             return elements_.at(mappings_.at(mem));
         } else {
             return {};
         }
    }

    std::string Group::str() const {
        std::stringstream s;

        s << "Group(";
        std::string sep = "";
        for (const auto& kv : getMappings()) {
            s << sep;
            s << kv.first << "=" << aovToString(kv.second);
            sep  = ", ";
        }

        s << ")";

        return s.str();
    }

    void Group::rotate() {
        std::lock_guard<std::mutex> guard(elements_mutex_);
        std::rotate(elements_.begin(), elements_.begin() + 1, elements_.end());
    }

    /* Commented out because I don't need it yet
    void Group::rotateIndex() {
        for (auto& kv : mappings_) {
            kv.second = kv.second + 1 % elements_.size();
        }
    }
    */

    AddressOrValue Group::exchange(const std::string& key, AddressOrValue aov) {
        std::lock_guard<std::mutex> guard(elements_mutex_);

        AddressOrValue old = elements_.at(mappings_.at(key));
        elements_[mappings_.at(key)] = aov;

        return old;
    }

    void Group::overwrite(const std::string& key, AddressOrValue aov) {
        std::lock_guard<std::mutex> guard(elements_mutex_);

        elements_[mappings_.at(key)] = aov;
    }

    std::map<std::string, AddressOrValue> Group::getMappings() const {
        std::map<std::string, AddressOrValue> mappings;

        for (const auto& kv : mappings_) {
            {
                std::lock_guard<std::mutex> guard(elements_mutex_);
                mappings[kv.first] = elements_.at(kv.second);
            }
        }

        return mappings;
    }

    void Group::setMapping(const std::string& key, int i) {
        mappings_[key] = i;
    }
}
