#include "fileutil.h"

// STL
#include <fstream>
#include <sstream>
#include <cstring>
#include <filesystem>

// Boost
#include <boost/filesystem.hpp>

namespace vidrevolt {
    namespace fileutil {
        std::string slurp(const std::string& path) {
            std::ifstream ifs(path);
            if (ifs.fail()) {
                std::ostringstream err;
                err << "Error loading " << path << " - " <<  std::strerror(errno);
                throw std::runtime_error(err.str());
            }

            std::stringstream stream;
            stream << ifs.rdbuf();

            return stream.str();
        }

        std::string slurp(const std::string& relative_to, const std::string& path) {
            return slurp(
                (boost::filesystem::path(relative_to).parent_path() / path).c_str()
            );
        }

        bool hasExtension(const std::string& path, const std::string& ext) {
            return path.size() >= ext.size() &&
                path.compare(path.size() - ext.size(), ext.size(), ext) == 0;

        }

        std::string join(const std::string& a, const std::string& b) {
            return (boost::filesystem::path(a) / boost::filesystem::path(b)).c_str();
        }
    }
}

