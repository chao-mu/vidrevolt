#include "Value.h"

// STL
#include <sstream>
#include <algorithm>

namespace vidrevolt {
    Value::Value() : Value(0.0f) {}
    Value::Value(bool v) : Value(v ? 1.0f : 0.0f) {}
    Value::Value(int v) : Value(static_cast<float>(v)) {}
    Value::Value(float v) : Value(std::vector({v})) {}

    Value::Value(std::vector<float> v) {
        if (v.empty()) {
            v.push_back(0);
        }

        while(v.size() < 4) {
            v.push_back(v.back());
        }

        std::copy_n(v.begin(), 4, value_.begin());
    }

    float Value::at(size_t i) const {
        return value_.at(i);
    }

    bool Value::getBool() const {
        return value_[0]  > 0.5 ? true : false;
    }

    int Value::getInt() const {
        return static_cast<int>(getFloat());
    }

    float Value::getFloat() const {
        return value_[0];
    }

    std::array<float, 4> Value::getVec4() const {
        return value_;
    }

    std::string Value::str() const {
        std::stringstream ss;

        std::string sep = "";
        for (const auto& v : value_) {
            ss << sep;
            ss << v;
            sep = ", ";
        }

        return ss.str();
    }
}
