#include "VideoWriter.h"

namespace vidrevolt {
    VideoWriter::VideoWriter(const std::string& path, double fps, const Resolution& res) :
        writer_(path, CV_FOURCC('M','J','P','G'), fps, cv::Size(res.width, res.height)) {}

    void VideoWriter::start() {
        running_ = true;
        thread_ = std::thread([this] {
            while (running_.load()) {
                if (!running_.load()) {
                    break;
                }

                {
                    std::unique_lock<std::mutex> lk(work_ready_mutex_);
                    work_ready_cv_.wait(lk, [this]{return work_.size() || !running_.load();});
                }

                while (work_.size()) {
                    cv::Mat frame = work_.front();
                    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
                    flip(frame, frame, 0);
                    writer_.write(frame);
                    work_.pop();
                }
            }
        });
    }

    void VideoWriter::write(cv::Mat frame) {
        {
            std::lock_guard lk(work_mutex_);
            work_.push(frame);
        }

        signalWork();
    }

    void VideoWriter::signalWork() {
        std::lock_guard lk(work_ready_mutex_);
        work_ready_cv_.notify_one();
    }


    void VideoWriter::close() {
        running_ = false;
        if (thread_.joinable()) {
            signalWork();
            thread_.join();
        }
    }
}
