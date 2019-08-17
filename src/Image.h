#ifndef FRAG_IMAGE_H_
#define FRAG_IMAGE_H_

// STL
#include <string>

// Ours
#include "Texture.h"

namespace frag {
    class Image : public Texture {
        public:
            Image(const std::string& path);
            void load();
            virtual std::string getPath() const override;
            static bool isImage(const std::string& path);

        private:
            std::string path_;
    };
}

#endif
