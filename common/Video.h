#ifndef FRAG_WEBCAM_H
#define FRAG_WEBCAM_H

// STL
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <string>
#include <condition_variable>

// OpenCV
#include <opencv2/opencv.hpp>

// Ours
#include "Media.h"

namespace vidrevolt {
    class Video : public Media {
        public:
            using Frame = std::pair<int, cv::Mat>;

            enum Playback {
                Mirror,
                Forward,
                Reverse,
                Once
            };

            ~Video();
            // Video(int device, double fps=0, cv::Size size=cv::Size(0,0));
            Video(const Address& addr, const std::string& path, bool auto_reset, Playback pb=Forward);
            Video(const std::string& path, Playback pb);

            void start();
            void stop();

            virtual std::optional<cv::Mat> nextFrame() override;
            std::optional<cv::Mat> nextFrame(bool force);

            virtual Resolution getResolution() override;

            virtual void outFocus() override;

            void setReverse(bool t);

            void flipPlayback();

            static bool isVideo(const std::string& path);

            virtual std::string getPath() const override;

            double getFPS() const;

        private:
            void next();
            void seek(int pos);
            Frame readFrame();
            void signalWork();

            // Constructor Parameters
            const std::string path_;
            size_t buffer_size_;
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

            int cursor_;
            std::vector<Frame> buffer_;

            bool work_ready_ = false;
            std::mutex work_ready_mutex_;
            std::condition_variable work_ready_cv_;

            Resolution res_;
    };
}

#endif
