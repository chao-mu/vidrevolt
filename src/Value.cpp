#include "Value.h"

// STL
#include <sstream>

namespace frag {
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

        value_ = v;
    }

    bool Value::getBool() {
        return value_[0]  > 0.5 ? true : false;
    }

    int Value::getInt() {
        return static_cast<int>(getFloat());
    }

    float Value::getFloat() {
        return value_[0];
    }

    std::vector<float> Value::getVec4() {
        return value_;
    }

    std::string Value::toString() const {
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
