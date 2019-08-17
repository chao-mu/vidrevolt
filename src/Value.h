#ifndef FRAG_VALUE_H_
#define FRAG_VALUE_H_

// STL
#include <vector>
#include <string>

namespace frag {
    class Value {
        public:
            Value();
            Value(bool v);
            Value(float v);
            Value(int v);
            Value(std::vector<float> v);

            bool getBool();
            float getFloat();
            int getInt();
            std::vector<float> getVec4();

            std::string str() const;

        private:
            std::vector<float> value_;
    };
}

#endif
