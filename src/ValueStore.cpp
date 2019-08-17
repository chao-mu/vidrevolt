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
        if (groups_.count(addr.getFront()) == 0) {
            return {};
        }

        std::shared_ptr<Group> group = groups_.at(addr.getFront());
        std::optional<AddressOrValue> aov_opt = group->get(addr.getBack());
        if (!aov_opt.has_value()) {
            return {};
        }

        AddressOrValue aov = aov_opt.value();

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
        if (images_.count(addr) > 0 ||
                videos_.count(addr) > 0 ||
                render_out_.count(addr) > 0 ||
                groups_.count(addr) > 0) {
            return addr;
        }

        if (aliases_.count(addr) > 0) {
            return getAddressDeep(aliases_.at(addr));
        }

        {
            std::lock_guard<std::mutex> guard(values_mutex_);
            if (values_.count(addr) > 0) {
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

        if (videos_.count(resolved_addr) > 0) {
            return videos_.at(resolved_addr);
        } else {
            return nullptr;
        }
    }

    std::shared_ptr<Group> ValueStore::getGroup(const Address& addr) const {
        Address resolved_addr = getAddressDeep(addr);

        if (groups_.count(resolved_addr) > 0) {
            return groups_.at(resolved_addr);
        } else {
            return nullptr;
        }
    }

    std::shared_ptr<Media> ValueStore::getMedia(const Address& addr) const {
        Address resolved_addr = getAddressDeep(addr);

        if (videos_.count(resolved_addr) > 0) {
            return videos_.at(resolved_addr);
        }

        if (images_.count(resolved_addr) > 0) {
            return images_.at(resolved_addr);
        }

        if (render_out_.count(resolved_addr) > 0) {
            return render_out_.at(resolved_addr);
        }

        return nullptr;
    }

    void ValueStore::set(Address alias, Address target) {
        aliases_[alias] = target;
    }

    void ValueStore::set(const Address& addr, std::shared_ptr<Texture> o) {
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
        videos_[addr] = v;
    }

    void ValueStore::set(const Address& addr, std::shared_ptr<Image> img) {
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
        for (const auto& kv : images_) {
            s << "    - " << kv.first.str() << ": " << kv.second->getPath() << std::endl;
        }

        s << "Videos:" << std::endl;
        for (const auto& kv : videos_) {
            s << "    - " << kv.first.str() << ": " << kv.second->getPath() << std::endl;
        }

        s << "Groups:" << std::endl;
        for (const auto& kv : groups_) {
            s << "    - " << kv.first.str() << ": " << kv.second->str() << std::endl;
        }

        s << "Aliases:" << std::endl;
        for (const auto& kv : aliases_) {
            s << "    - " << kv.first.str() << ": " << kv.second.str();
            s << " (getAddressDeep -> " << getAddressDeep(kv.first).str() << ")" << std::endl;
        }

        s << "Render outputs:" << std::endl;
        for (const auto& kv : render_out_) {
            s << "    - " << kv.first.str() << std::endl;
        }

        s << "--- ValueStore END ---";

        return s.str();
    }
}
