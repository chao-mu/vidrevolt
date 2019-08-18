#ifndef FRAG_FILEUTIL_H_
#define FRAG_FILEUTIL_H_

#include <string>

namespace vidrevolt {
    namespace fileutil {
        std::string slurp(const std::string& path);
        std::string slurp(const std::string& relative_to, const std::string& path);
        std::string join(const std::string& a, const std::string& b);
        bool hasExtension(const std::string& path, const std::string& ext);
    }
}
#endif
