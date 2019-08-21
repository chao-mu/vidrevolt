#ifndef FRAG_IMAGE_H_
#define FRAG_IMAGE_H_

// STL
#include <string>

// Ours
#include "Media.h"

namespace vidrevolt {
    class Image : public Media {
        public:
            Image(const Address& addr, const std::string& path);
            virtual std::string getPath() const override;
            static bool isImage(const std::string& path);

            void load();

            virtual std::optional<cv::Mat> nextFrame() override;

            virtual Resolution getResolution() override;

        private:
            std::string path_;
            cv::Mat image_;
            bool new_data_ = false;
            Resolution res_;

    };
}

#endif
