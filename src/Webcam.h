#ifndef VIDREVOLT_WEBCAM_H_
#define VIDREVOLT_WEBCAM_H_

// STL
#include <memory>
#include <optional>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>

// OpenCV
#include <opencv2/opencv.hpp>

// Ours
#include "FrameSource.h"

namespace vidrevolt {
    class Webcam : public FrameSource {
        public:
            Webcam(int device);

            virtual ~Webcam() = default;

            void start();

            std::optional<cv::Mat> nextFrame() override;

        private:
            void signalWork();
            void work();

            cv::Mat frame_;

            std::unique_ptr<cv::VideoCapture> vid_;

            const int device_;

            std::thread thread_;
            std::atomic<bool> running_ = false;
            std::mutex frame_mutex_;

            bool work_ready_ = false;
            std::mutex work_ready_mutex_;
            std::condition_variable work_ready_cv_;
    };
}
#endif
