#include "ValueStore.h"

// STL
#include <iostream>

namespace vidrevolt {
    bool ValueStore::isMedia(const Address& addr) const {
        if (is_media_.count(addr) > 0) {
            return is_media_.at(addr);
        } else {
            return getMedia(addr) != nullptr;
        }
    }

    Address ValueStore::resolveAlias(const Address& addr_in, int depth) {
        if (depth < 0) {
            throw std::runtime_error("Recursive aliases found for address " + addr_in.str());
        }

        if (aliases_.count(addr_in) > 0) {
            return resolveAlias(aliases_.at(addr_in), depth - 1);
        }

        return addr_in;
    }

    void ValueStore::setGroupMember(const Address& addr_in, AddressOrValue aov) {
        Address addr = resolveAlias(addr_in);

        std::shared_ptr<Group> group = getGroup(addr.getFront());
        if (group == nullptr) {
            throw std::runtime_error("Expected " + addr_in.str() + "(" + addr.str() +
                    ") to be a group member.");
        }

        group->overwrite(addr.getBack(), aov);
    }

    std::optional<AddressOrValue> ValueStore::getGroupMember(const Address& addr) const {
        std::shared_ptr<Group> group;
        {
            std::lock_guard<std::mutex> guard(groups_mutex_);
            Address group_addr = addr.getFront();
            if (groups_.count(group_addr) > 0) {
                group = groups_.at(group_addr);
            } else {
                return {};
            }
        }
        if (group == nullptr) {
            return {};
        }

        AddressOrValue aov;
        std::optional<AddressOrValue> aov_opt = group->get(addr.getBack());
        if (!aov_opt.has_value()) {
            return {};
        }

        aov = aov_opt.value();

        // Re-add swizzle
        if (isAddress(aov)) {
            Address member = std::get<Address>(aov);
            std::string swiz = addr.getSwiz();

            if (swiz != "") {
                member.setSwiz(swiz);
            }

            return member;
        }

        return aov;
    }

    Address ValueStore::getAddressDeep(const Address& addr) const {
        bool is_alias;
        Address alias_target;
        {
            std::lock_guard<std::mutex> guard(aliases_mutex_);
            is_alias = aliases_.count(addr) > 0;
            if (is_alias) {
                alias_target = aliases_.at(addr);
            }
        }
        if (is_alias) {
            return getAddressDeep(alias_target);
        }

        {
            std::lock_guard<std::mutex> guard(values_mutex_);
            if (values_.count(addr) > 0) {
                return addr;
            }
        }

        {
            std::lock_guard<std::mutex> guard(groups_mutex_);
            if (groups_.count(addr) > 0) {
                return addr;
            }
        }

        {
            std::lock_guard<std::mutex> guard(images_mutex_);
            if (images_.count(addr) > 0) {
                return addr;
            }
        }

        {
            std::lock_guard<std::mutex> guard(videos_mutex_);
            if (videos_.count(addr) > 0) {
                return addr;
            }
        }

        // Maybe it's a group member
        std::optional<AddressOrValue> member_opt = getGroupMember(addr);
        if (member_opt.has_value()) {
            AddressOrValue aov = member_opt.value();
            if (isAddress(aov)) {
                return getAddressDeep(std::get<Address>(aov));
            } else {
                return addr;
            }
        }

        return addr;
    }

    std::optional<Value> ValueStore::getValue(const Address& addr) const {
        Address resolved_addr = getAddressDeep(addr);

        {
            std::lock_guard<std::mutex> guard(values_mutex_);
            if (values_.count(resolved_addr)) {
                return values_.at(resolved_addr);
            }
        }

        std::optional<AddressOrValue> aov_opt = getGroupMember(resolved_addr);
        if (aov_opt.has_value() && isValue(aov_opt.value())) {
            return std::get<Value>(aov_opt.value());
        }

        if (resolved_addr.getBack() == "resolution") {
            std::shared_ptr<Media> media = getMedia(resolved_addr.withoutBack());
            if (media != nullptr) {
                Resolution res = media->getResolution();
                return Value(std::vector({static_cast<float>(res.width), static_cast<float>(res.height)}));
            } else {
                std::lock_guard<std::mutex> guard(values_mutex_);
                return values_.at("resolution");
            }
        }

        return {};
    }

    void ValueStore::setIsMedia(Address addr, bool t) {
        std::lock_guard<std::mutex> guard(is_media_mutex_);

        is_media_[addr] = t;
    }

    std::shared_ptr<Video> ValueStore::getVideo(const Address& addr) const {
        Address resolved_addr = getAddressDeep(addr);

        std::lock_guard<std::mutex> guard(videos_mutex_);
        if (videos_.count(resolved_addr) > 0) {
            return videos_.at(resolved_addr);
        } else {
            return nullptr;
        }
    }

    std::shared_ptr<Group> ValueStore::getGroup(const Address& addr) const {
        Address resolved_addr = getAddressDeep(addr);

        std::lock_guard<std::mutex> guard(groups_mutex_);
        if (groups_.count(resolved_addr) > 0) {
            return groups_.at(resolved_addr);
        } else {
            return nullptr;
        }
    }

    std::shared_ptr<Media> ValueStore::getMedia(const Address& addr) const {
        Address resolved_addr = getAddressDeep(addr);

        {
            std::lock_guard<std::mutex> guard(videos_mutex_);
            if (videos_.count(resolved_addr) > 0) {
                return videos_.at(resolved_addr);
            }
        }

        {
            std::lock_guard<std::mutex> guard(images_mutex_);
            if (images_.count(resolved_addr) > 0) {
                return images_.at(resolved_addr);
            }
        }


        return nullptr;
    }

    void ValueStore::set(Address alias, Address target) {
        std::lock_guard<std::mutex> guard(aliases_mutex_);
        aliases_[alias] = target;
    }

    void ValueStore::set(const Address& addr, std::shared_ptr<Group> g) {
        groups_[addr] = g;
    }

    void ValueStore::set(const Address& addr, Value v) {
        std::lock_guard<std::mutex> guard(values_mutex_);
        values_[addr] = v;
    }

    void ValueStore::set(const Address& addr, std::shared_ptr<Video> v) {
        std::lock_guard<std::mutex> guard(videos_mutex_);
        videos_[addr] = v;
    }

    void ValueStore::set(const Address& addr, std::shared_ptr<Image> img) {
        std::lock_guard<std::mutex> guard(images_mutex_);
        images_[addr] = img;
    }

    void ValueStore::set(const Address& addr, std::shared_ptr<Controller> c) {
        for (const auto ctrl_name : c->getControlNames()) {
            Address ctrl_addr = addr + ctrl_name;
            c->connect(ctrl_name, [ctrl_addr, this](Value v) {
                set(ctrl_addr, v);
            });
        }

        controllers_[addr] = c;
    }

    std::map<Address, std::shared_ptr<Media>> ValueStore::getMediaAll() const {
        std::map<Address, std::shared_ptr<Media>> media;

        {
            std::lock_guard<std::mutex> guard(images_mutex_);
            media.insert(images_.cbegin(), images_.cend());
        }

        {
            std::lock_guard<std::mutex> guard(videos_mutex_);
            media.insert(videos_.cbegin(), videos_.cend());
        }

        return media;
    }

    std::string ValueStore::str() const {
        std::stringstream s;

        s << "--- ValueStore START ---" << std::endl;

        s << "Values:" << std::endl;
        {
            std::lock_guard<std::mutex> guard(values_mutex_);
            for (const auto& kv : values_) {
                s << "    - " << kv.first.str() << ": " << kv.second.str() << std::endl;
            }
        }

        s << "Images:" << std::endl;
        {
            std::lock_guard<std::mutex> guard(images_mutex_);
            for (const auto& kv : images_) {
                s << "    - " << kv.first.str() << ": " << kv.second->getPath() << std::endl;
            }
        }

        s << "Videos:" << std::endl;
        {
            std::lock_guard<std::mutex> guard(videos_mutex_);
            for (const auto& kv : videos_) {
                s << "    - " << kv.first.str() << ": " << kv.second->getPath() << std::endl;
            }
        }

        s << "Groups:" << std::endl;
        {
            std::lock_guard<std::mutex> guard(groups_mutex_);
            for (const auto& kv : groups_) {
                s << "    - " << kv.first.str() << ": " << kv.second->str() << std::endl;
            }
        }

        s << "Aliases:" << std::endl;
        {
            std::lock_guard<std::mutex> guard(aliases_mutex_);
            for (const auto& kv : aliases_) {
                s << "    - " << kv.first.str() << ": " << kv.second.str() << std::endl;
            }
        }

        s << "--- ValueStore END ---";

        return s.str();
    }
}
