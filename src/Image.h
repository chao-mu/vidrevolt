#ifndef FRAG_IMAGE_H_
#define FRAG_IMAGE_H_

// STL
#include <string>

// OpenCV
#include <opencv2/opencv.hpp>

namespace vidrevolt {
    class Image {
        public:
            static cv::Mat load(const std::string& path);

        private:
            Image();
    };
}

#endif
