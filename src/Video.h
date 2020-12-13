#ifndef FRAG_WEBCAM_H
#define FRAG_WEBCAM_H

// STL
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <string>
#include <condition_variable>
#include <future>

// OpenCV
#include <opencv2/opencv.hpp>

// Ours
#include "Resolution.h"

#define VIDREVOLT_VIDEO_MIDDLE 15
#define VIDREVOLT_VIDEO_BUFFER_SIZE 30

namespace vidrevolt {
    class Video {
        public:
            using Frame = std::pair<int, cv::Mat>;

            enum Playback {
                Mirror,
                Forward,
                Reverse,
                Once
            };

            ~Video();
            Video(const std::string& path, bool auto_reset, Playback pb=Forward);
            Video(const std::string& path, Playback pb);

            void start();
            void stop();

            std::optional<cv::Mat> nextFrame();
            std::optional<cv::Mat> nextFrame(bool force);

            Resolution getResolution();

            void outFocus();
            void inFocus();

            void setReverse(bool t);

            void flipPlayback();

            std::string getPath() const;

            std::optional<Frame> currentFrame();

            void setFPS(double fps);
            double getFPS() const;

            double getRemainingMS();

            void waitForLoaded();

        private:
            void next();
            void seek(int pos);
            Frame readFrame();
            void signalWork();

            double length_ms = 0;

            // Constructor Parameters
            const std::string path_;
            std::atomic<bool> reverse_ = false;
            bool auto_reset_;
            const Playback playback_;
            bool finished_ = false;

            std::optional<std::chrono::high_resolution_clock::time_point> last_update_;
            std::mutex buffer_mutex_;
            std::unique_ptr<cv::VideoCapture> vid_;
            std::thread thread_;
            std::atomic<bool> running_ = false;
            double fps_ = 0;

            std::atomic<int> last_frame_ = 0;
            int total_frames_ = 0;

            std::atomic<bool> requested_reset_ = false;

            int cursor_ = 0;
            std::vector<Frame> buffer_;

            bool work_ready_ = false;
            std::mutex work_ready_mutex_;
            std::condition_variable work_ready_cv_;

            Resolution res_;

            float error_ = 0;

            std::mutex load_mutex_;
    };
}

#endif
