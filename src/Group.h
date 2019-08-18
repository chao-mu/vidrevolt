#ifndef FRAG_GROUP_H_
#define FRAG_GROUP_H_

// STL
#include <map>
#include <mutex>

// Ours
#include "AddressOrValue.h"

namespace vidrevolt {
    class Group {
        public:
            std::map<std::string, AddressOrValue> getMappings() const;
            void setMapping(const std::string& key, int i);

            void rotate();
            AddressOrValue exchange(const std::string& key, AddressOrValue aov);
            void overwrite(const std::string& key, AddressOrValue aov);
            void add(AddressOrValue aov);
            std::optional<AddressOrValue> get(std::string mem) const;
            std::string str() const;

        private:
            std::vector<AddressOrValue> elements_;
            std::map<std::string, int> mappings_;
            mutable std::mutex elements_mutex_;
    };
}

#endif
