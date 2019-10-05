#ifndef FRAG_FILEUTIL_H_
#define FRAG_FILEUTIL_H_

// STL
#include <string>
#include <vector>
#include <functional>

namespace vidrevolt {
    namespace fileutil {
        std::string slurp(const std::string& path);
        std::string slurp(const std::string& relative_to, const std::string& path);
        std::string join(const std::string& a, const std::string& b);

        void per_line(const std::string& path, std::function<void(std::string)> f);
    }
}
#endif
