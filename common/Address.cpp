#include "Address.h"

#define VIDREVOLT_ADDRESS_SEP "."

namespace vidrevolt {
    Address::Address(const std::vector<std::string>& fields) : fields_(fields) {}
    Address::Address(const std::vector<std::string>& fields, const std::string& tail) : fields_(fields) {
        fields_.push_back(tail);
    }

    size_t Address::getDepth() const {
        return fields_.size();
    }

    std::string Address::str() const {
        std::string str;

        std::string sep = "";
        for (const auto& field : fields_) {
            // TODO: Remove PatchParser call to our constructor with empty values. Boo
            if (field.empty()) {
                continue;
            }

            str += sep;
            str += field;
            sep = VIDREVOLT_ADDRESS_SEP;
        }

        return str;
    }

    Address Address::withoutBack() const {
        if (fields_.empty()) {
            return Address();
        }

        return Address(std::vector(fields_.cbegin(), fields_.cend() - 1));
    }

    Address Address::withoutFront() const {
        if (fields_.empty()) {
            return Address();
        }

        return Address(std::vector(fields_.cbegin() + 1, fields_.cend()));
    }

    Address Address::withHead(const std::string head) const {
        std::vector<std::string> fields(fields_.cbegin(), fields_.cend());
        fields.insert(fields.begin(), head);
        return Address(fields);
    }

    std::string Address::getSwiz() const {
        return swiz_;
    }

    void Address::setSwiz(const std::string& str) {
        swiz_ = str;
    }

    std::string Address::getBack() const {
        if (fields_.empty()) {
            return "";
        }

        return fields_.back();
    }

    std::string Address::getFront() const {
        if (fields_.empty()) {
            return "";
        }

        return fields_.front();
    }

    std::vector<std::string> Address::getFields() const {
        return fields_;
    }

    Address Address::operator+(const Address& addr) const {
        return Address(this->getFields(), addr.str());
    }

    // This can often be seen written as
    bool Address::operator<(const Address& b) const {
        return str() < b.str();
    }
}
