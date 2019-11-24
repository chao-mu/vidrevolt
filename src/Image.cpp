#include "Image.h"

namespace vidrevolt {
    cv::Mat Image::load(const std::string& path) {
        cv::Mat image = cv::imread(path);
        if (image.empty()) {
            throw std::runtime_error("Unable to load image " + path);
        }

        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
        flip(image, image, 0);

        return image;
    }
}
