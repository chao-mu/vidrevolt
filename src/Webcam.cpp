#include "Webcam.h"

#include "debug.h"
#define debug_time false

DEBUG_TIME_DECLARE(nextFrame)

namespace vidrevolt {
    Webcam::Webcam(int device) : device_(device) {}

    std::optional<cv::Mat> Webcam::nextFrame() {
        DEBUG_TIME_START(nextFrame)

        cv::Mat frame;
        {
            std::lock_guard lk(frame_mutex_);
            frame_.copyTo(frame);
        }

        signalWork();

        DEBUG_TIME_END(nextFrame)

        if (frame.empty()) {
            return {};
        } else {
            return frame;
        }
    }

    void Webcam::work() {
        cv::Mat tmp_frame;
        vid_->read(tmp_frame);
        if (!tmp_frame.empty()) {
            cv::cvtColor(tmp_frame, tmp_frame, cv::COLOR_BGR2RGB);
            flip(tmp_frame, tmp_frame, 0);
        }

        {
            std::lock_guard lk(frame_mutex_);
            tmp_frame.copyTo(frame_);
        }
    }

    void Webcam::start() {
        //vid_ = std::make_unique<cv::VideoCapture>(device_, cv::CAP_V4L2);
        vid_ = std::make_unique<cv::VideoCapture>(device_);
        if (!vid_->isOpened()) {
            throw std::runtime_error("Unable to open capture device " + std::to_string(device_));
        }

        work();

        running_ = true;

        thread_ = std::thread([this] {
            //DEBUG_TIME_DECLARE(work_wait)
            while (running_.load()) {
                {
                    //DEBUG_TIME_START(work_wait)
                    std::unique_lock<std::mutex> lk(work_ready_mutex_);
                    work_ready_cv_.wait(lk, [this]{return work_ready_ || !running_.load();});
                    //DEBUG_TIME_END(work_wait)
                }

                if (!running_.load()) {
                    break;
                }

                work();

                {
                    std::lock_guard lk(work_ready_mutex_);
                    work_ready_ = false;
                }
            }
        });
    }

    void Webcam::signalWork() {
        std::lock_guard lk(work_ready_mutex_);
        work_ready_ = true;
        work_ready_cv_.notify_one();
    }
}
