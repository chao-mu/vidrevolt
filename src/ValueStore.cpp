#include "ValueStore.h"

// STL
#include <iostream>
#include <sstream>

// Ours
#include "MathUtil.h"

namespace frag {
    bool ValueStore::isMedia(const Address& addr) const {
        return getMedia(addr) != nullptr;
    }

    std::optional<AddressOrValue> ValueStore::getGroupMember(const Address& addr) const {
        {
            std::lock_guard<std::mutex> guard(groups_mutex_);
            if (groups_.count(addr.getFront()) == 0) {
                return {};
            }
        }

        AddressOrValue aov;
        {
            std::lock_guard<std::mutex> guard(groups_mutex_);
            std::shared_ptr<Group> group = groups_.at(addr.getFront());
            std::optional<AddressOrValue> aov_opt = group->get(addr.getBack());
            if (!aov_opt.has_value()) {
                return {};
            }
            aov = aov_opt.value();
        }


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

        {
            std::lock_guard<std::mutex> guard(render_out_mutex_);
            if (render_out_.count(addr) > 0) {
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
                return frag::Value(
                        std::vector({static_cast<float>(res.width), static_cast<float>(res.height)}));

            }
        }

        return {};
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

        {
            std::lock_guard<std::mutex> guard(render_out_mutex_);
            if (render_out_.count(resolved_addr) > 0) {
                return render_out_.at(resolved_addr);
            }
        }

        return nullptr;
    }

    void ValueStore::set(Address alias, Address target) {
        std::lock_guard<std::mutex> guard(aliases_mutex_);
        aliases_[alias] = target;
    }

    void ValueStore::set(const Address& addr, std::shared_ptr<Texture> o) {
        std::lock_guard<std::mutex> guard(render_out_mutex_);
        render_out_[addr] = o;
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

    void ValueStore::set(const Address& addr, std::shared_ptr<midi::Device> d) {
        for (const auto ctrl_name : d->getControlNames()) {
            Address ctrl_addr = addr + ctrl_name;
            d->connect(ctrl_name, [ctrl_addr, this](Value v) {
                set(ctrl_addr, v);
            });
        }

        midi_devices_[addr] = d;
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

        s << "Render outputs:" << std::endl;
        {
            std::lock_guard<std::mutex> guard(render_out_mutex_);
            for (const auto& kv : render_out_) {
                s << "    - " << kv.first.str() << std::endl;
            }
        }

        s << "--- ValueStore END ---";

        return s.str();
    }
}
