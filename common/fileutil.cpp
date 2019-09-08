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

        bool hasExtension(const std::string& path, const std::vector<std::string> exts) {
            for (const auto& ext : exts) {
                if (fileutil::hasExtension(path, ext)) {
                    return true;
                }
            }

            return false;
        }

        std::string join(const std::string& a, const std::string& b) {
            return (boost::filesystem::path(a) / boost::filesystem::path(b)).c_str();
        }

        bool hasImageExt(const std::string& path) {
            return hasExtension(path, {
                "ase", "art", "bmp", "blp", "cd5", "cit", "cpt", "cr2", "cut", "dds",
                "dib", "djvu", "egt", "exif", "gif", "gpl", "grf", "icns", "ico", "iff",
                "jng", "jpeg", "jpg", "jfif", "jp2", "jps", "lbm", "max", "miff", "mng",
                "msp", "nitf", "ota", "pbm", "pc1", "pc2", "pc3", "pcf", "pcx", "pdn",
                "pgm", "PI1", "PI2", "PI3", "pict", "pct", "pnm", "pns", "ppm", "psb",
                "psd", "pdd", "psp", "px", "pxm", "pxr", "qfx", "raw", "rle", "sct",
                "sgi", "rgb", "int", "bw", "tga", "tiff", "tif", "vtf", "xbm", "xcf",
                "xpm", "3dv", "amf", "ai", "awg", "cgm", "cdr", "cmx", "dxf", "e2d",
                "egt", "eps", "fs", "gbr", "odg", "svg", "stl", "vrml", "x3d", "sxd",
                "v2d", "vnd", "wmf", "emf", "art", "xar", "png", "webp", "jxr", "hdp",
                "wdp", "cur", "ecw", "iff", "lbm", "liff", "nrrd", "pam", "pcx", "pgf",
                "sgi", "rgb", "rgba", "bw", "int", "inta", "sid", "ras", "sun", "tga"
            });
        }

        bool hasVideoExt(const std::string& path) {
            return hasExtension(path, {
                "3g2", "3gp", "aaf", "asf", "avchd", "avi", "drc", "flv", "m2v",
                "m4p", "m4v", "mkv", "mng", "mov", "mp2", "mp4", "mpe", "mpeg", "mpg",
                "mpv", "mxf", "nsv", "ogg", "ogv", "qt", "rm", "rmvb", "roq", "svi",
                "vob", "webm", "wmv", "yuv", "ivf"
            });
        }
    }
}
