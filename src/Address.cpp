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

        return Address(std::vector<std::string>(fields_.cbegin() + 1, fields_.cend()));
    }

    Address Address::withHead(const std::string& head) const {
        std::vector<std::string> fields(fields_.cbegin(), fields_.cend());
        fields.insert(fields.begin(), head);
        return Address(fields);
    }

    void Address::setSwiz(const std::string& swiz_in) {
        std::string swiz = swiz_in;

        // Ensure non-empty
        if (swiz.empty()) {
            swiz = "xyzw";
        }

        // Ensure at least 4 characters
        while (swiz.length() < 4) {
            swiz += swiz.back();
        }

        // Ensure not more than 4 characters
        while (swiz.length() > 4) {
            swiz.pop_back();
        }

        // Translate characters to indices
        for (size_t i = 0; i < 4; i++) {
            char c = swiz.at(i);
            if (c == 'x' || c == 'r') {
                swiz_[i] = 0;
            } else if (c == 'y' || c == 'g') {
                swiz_[i] = 1;
            } else if (c == 'z' || c == 'b') {
                swiz_[i] = 2;
            } else if (c == 'w' || c == 'a') {
                swiz_[i] = 3;
            } else {
                throw std::runtime_error("Unexpected character found in swizzle '" + swiz_in + "'");
            }
        }
    }

    void Address::setSwiz(const std::array<int, 4>& swiz) {
        swiz_ = swiz;
    }

    std::array<int, 4> Address::getSwiz() const {
        return swiz_;
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
