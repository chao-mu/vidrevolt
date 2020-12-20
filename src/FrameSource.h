#ifndef VIDREVOLT_FRAMESOURCE_H_
#define VIDREVOLT_FRAMESOURCE_H_

// STL
#include <optional>

// OpenCV
#include <opencv2/opencv.hpp>

namespace vidrevolt {
    class FrameSource {
        public:
            virtual std::optional<cv::Mat> nextFrame() = 0;
    };
}
#endif
