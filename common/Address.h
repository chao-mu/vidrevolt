#ifndef FRAG_ADDRESS_H_
#define FRAG_ADDRESS_H_

// STL
#include <string>
#include <vector>

namespace vidrevolt {
    class Address {
        public:
            Address(const std::vector<std::string>& fields);
            Address(const std::vector<std::string>& fields, const std::string& tail);
            template<class ...Ts> Address(Ts... fields) : fields_{fields...} {}
            //template<class ...Ts> Address(Ts... fields, const std::string& last) : fields_{fields..., last} {}

            size_t getDepth() const;

            Address withoutBack() const;
            Address withoutFront() const;

            Address withHead(const std::string head) const;

            std::string getFront() const;
            std::string getBack() const;

            void setSwiz(const std::string& str);
            std::string getSwiz() const;

            bool operator <(const Address& b) const;
            Address operator +(const std::string& str) const;

            std::string str() const;

            std::vector<std::string> getFields() const;
        private:
            std::vector<std::string> fields_;
            std::string swiz_;
    };
}
#endif
