#ifndef FRAG_VALUE_H_
#define FRAG_VALUE_H_

// STL
#include <vector>
#include <string>
#include <array>

namespace vidrevolt {
    class Value {
        public:
            Value();
            Value(bool v);
            Value(float v);
            Value(int v);
            Value(std::vector<float> v);

            float at(size_t i) const;
            bool getBool() const;
            float getFloat() const;
            int getInt() const;
            std::array<float, 4> getVec4() const;

            std::string str() const;

        private:
            std::array<float, 4> value_;
    };
}

#endif
