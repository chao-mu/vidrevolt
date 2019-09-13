#ifndef VIDREVOLT_VIDEOWRITER_H_
#define VIDREVOLT_VIDEOWRITER_H_

// STL
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>

// OpenCV
#include "opencv2/opencv.hpp"

// Ours
#include "Resolution.h"

namespace vidrevolt {
    class VideoWriter {
        public:
            VideoWriter(const std::string& path, double fps, const Resolution& res);

            void start();
            void write(cv::Mat frame);
            void close();

        private:
            void signalWork();

            cv::VideoWriter writer_;

            std::queue<cv::Mat> work_;
            std::thread thread_;
            mutable std::mutex work_mutex_;

            std::mutex work_ready_mutex_;
            std::condition_variable work_ready_cv_;
            std::atomic<bool> running_ = false;

    };
}

#endif

