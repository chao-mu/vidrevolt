#include "AddressOrValue.h"

namespace frag {
    bool isAddress(const AddressOrValue& aov) {
        return std::holds_alternative<Address>(aov);
    }

    bool isValue(const AddressOrValue& aov) {
        return std::holds_alternative<Value>(aov);
    }

    std::string aovToString(const AddressOrValue& aov) {
        std::string s = "AddressOrValue(";
        if (isValue(aov)) {
             s += std::get<Value>(aov).toString();
        } else if (isAddress(aov)) {
            s += std::get<Address>(aov).toString();
        } else {
            s += "undefined";
        }

        s += ")";

        return s;
    }
}
