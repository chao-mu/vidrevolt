#include "fileutil.h"

// STL
#include <fstream>
#include <sstream>
#include <cstring>

// Boost
#include <boost/filesystem.hpp>

namespace vidrevolt::fileutil {
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


    void per_line(const std::string& path, std::function<void(std::string)> f) {
        std::ifstream controls_file(path);
        for (std::string line; getline(controls_file, line);) {
            f(line);
        }
    }

    std::string slurp(const std::string& relative_to, const std::string& path) {
        return slurp(
                (boost::filesystem::path(relative_to).parent_path() / path).c_str()
                );
    }

    std::string join(const std::string& a, const std::string& b) {
        return (boost::filesystem::path(a) / boost::filesystem::path(b)).c_str();
    }
}
