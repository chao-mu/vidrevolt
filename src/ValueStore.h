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
#include "midi/Control.h"
#include "Trigger.h"

namespace frag {
    class ValueStore {
        public:
            bool isMedia(Address addr) const;
            void setIsMedia(Address addr, bool is_media);

            std::optional<Value> getValue(Address addr) const;
            std::shared_ptr<Media> getMedia(Address addr) const;
            std::shared_ptr<Video> getVideo(Address addr) const;
            std::shared_ptr<Group> getGroup(Address addr) const;

            void set(Address addr, Value v);
            void set(Address addr, midi::Control c);
            void set(Address addr, std::shared_ptr<Group> g);
            void set(Address addr, std::shared_ptr<Video> v);
            void set(Address addr, std::shared_ptr<Texture> t);
            void set(Address alias, Address target);

            Address getAddress(Address addr) const;

            bool isTriggered(const Trigger& trigger) const;

            std::string toString() const;

        private:
            void setMedia(Address addr, std::shared_ptr<Media> m);

            std::map<Address, Value> values_;
            std::map<Address, std::shared_ptr<Media>> media_;
            std::map<Address, bool> is_media_;
            std::map<Address, AddressOrValue> aovs_;
            std::map<Address, std::shared_ptr<Group>> groups_;
            std::map<Address, std::shared_ptr<Video>> videos_;
            std::map<Address, Address> aliases_;
    };
}

#endif
