#include "Image.h"

#include "fileutil.h"

namespace vidrevolt {
    Image::Image(const std::string& path) : path_(path) {}

    void Image::load() {
        image_ = cv::imread(path_);
        if (image_.empty()) {
            throw std::runtime_error("Unable to load image " + path_);
        }

        cv::cvtColor(image_, image_, cv::COLOR_BGR2RGB);
        flip(image_, image_, 0);

        res_.width = image_.size().width;
        res_.height = image_.size().height;

        new_data_ = true;
    }

    Resolution Image::getResolution() {
        return res_;
    }

    std::optional<cv::Mat> Image::nextFrame() {
        setInUse(true);

        if (new_data_) {
            new_data_ = false;
            return image_;
        } else {
            return {};
        }
     }

    std::string Image::getPath() const {
        return path_;
    }
}
