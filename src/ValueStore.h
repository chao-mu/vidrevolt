#ifndef FRAG_VALUESTORE_H_
#define FRAG_VALUESTORE_H_

// STL
#include <optional>
#include <map>
#include <memory>

// Ours
#include "AddressOrValue.h"
#include "Group.h"
#include "Media.h"
#include "Video.h"
#include "Image.h"
#include "Controller.h"
#include "Trigger.h"
#include "gl/Texture.h"

namespace vidrevolt {
    class ValueStore {
        public:
            bool isMedia(const Address& addr) const;

            std::optional<Value> getValue(const Address& addr) const;
            std::shared_ptr<Video> getVideo(const Address& addr) const;
            std::shared_ptr<Image> getImage(const Address& addr) const;
            std::shared_ptr<Group> getGroup(const Address& addr) const;
            std::shared_ptr<Media> getMedia(const Address& addr) const;
            std::optional<AddressOrValue> getGroupMember(const Address& addr) const;

            void set(const Address& addr, Value v);
            void set(const Address& addr, std::shared_ptr<Group> g);
            void set(const Address& addr, std::shared_ptr<Video> v);
            void set(const Address& addr, std::shared_ptr<Image> t);
            void set(const Address& addr, std::shared_ptr<gl::Texture> r);
            void set(const Address& addr, std::shared_ptr<Controller> c);
            void set(Address alias, Address target);
            void setGroupMember(const Address& addr, AddressOrValue aov);

            void setIsMedia(Address addr, bool t);

            Address getAddressDeep(const Address& addr) const;
            Address resolveAlias(const Address& addr_in, int depth=50);

            std::string str() const;

            std::map<Address, std::shared_ptr<Media>> getMediaAll() const;
        private:
            std::map<Address, Value> values_;
            std::map<Address, std::shared_ptr<Group>> groups_;
            std::map<Address, std::shared_ptr<Image>> images_;
            std::map<Address, std::shared_ptr<Video>> videos_;
            std::map<Address, std::shared_ptr<Controller>> controllers_;
            std::map<Address, Address> aliases_;
            std::map<Address, bool> is_media_;

            mutable std::mutex is_media_mutex_;
            mutable std::mutex values_mutex_;
            mutable std::mutex groups_mutex_;
            mutable std::mutex images_mutex_;
            mutable std::mutex videos_mutex_;
            mutable std::mutex aliases_mutex_;
    };
}

#endif
