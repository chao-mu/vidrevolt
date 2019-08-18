#include "Media.h"

namespace vidrevolt {
    void Media::update() {
        in_use_ = true;
    }

    std::string Media::getPath() const {
        return "";
    }

    bool Media::isInUse() {
        return in_use_;
    }

    bool Media::wasInUse() {
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
