#include "Media.h"

namespace vidrevolt {
    Media::Media(const Address& addr) : address_(addr) {}

    std::string Media::getPath() const {
        return "";
    }

    Address Media::getAddress() const {
        return address_;
    }

    void Media::setInUse(bool t) {
        in_use_ = t;
    }

    bool Media::isInUse() const {
        return in_use_;
    }

    bool Media::wasInUse() const {
        return last_in_use_;
    }

    void Media::inFocus() {
    }

    void Media::outFocus() {
    }

    void Media::resetInUse() {
        last_in_use_ = in_use_;
        in_use_ = false;
    }
}
