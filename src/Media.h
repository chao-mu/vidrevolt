#ifndef FRAG_MEDIA_H_
#define FRAG_MEDIA_H_

// STL
#include <string>

// OpenCV
#include <opencv2/opencv.hpp>

// Ours
#include "Resolution.h"
#include "Address.h"

namespace vidrevolt {
    class Media {
        public:
            Media(const Address& addr);

            virtual Resolution getResolution() = 0;
            virtual void inFocus();
            virtual void outFocus();
            virtual std::string getPath() const;

            bool isInUse() const;
            bool wasInUse() const;
            void resetInUse();
            void setInUse(bool t);

            Address getAddress() const;

            virtual std::optional<cv::Mat> nextFrame() = 0;

        private:
            bool in_use_ = false;
            bool last_in_use_ = false;
            Address address_;
    };
}
#endif
