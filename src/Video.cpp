#include "Video.h"

// STL
#include <algorithm>
#include <stdexcept>

// Ours
#include "fileutil.h"

#include "debug.h"
#define debug_time false

#define WORK_THRESHOLD 3

DEBUG_TIME_DECLARE(seek)
DEBUG_TIME_DECLARE(read_rev)
DEBUG_TIME_DECLARE(read_single)
DEBUG_TIME_DECLARE(frame_processing)

namespace vidrevolt {
    Video::Video(const std::string& path, Playback pb) : Video(path, false, pb) {}

    Video::Video(const std::string& path, bool auto_reset, Playback pb) :
        path_(path), reverse_(pb == Reverse),
        auto_reset_(auto_reset), playback_(pb) {}

    Video::~Video() {
        stop();
    }

    std::string Video::getPath() const {
        return path_;
    }

    void Video::inFocus() {

    }

    void Video::outFocus() {
        requested_reset_ = auto_reset_;
        last_update_.reset();
        signalWork();
    }

    void Video::signalWork() {
        std::lock_guard lk(work_ready_mutex_);
        work_ready_ = true;
        work_ready_cv_.notify_one();
    }

    std::optional<cv::Mat> Video::nextFrame() {
        return nextFrame(false);
    }

    std::optional<cv::Mat> Video::nextFrame(bool force) {
        if (finished_) {
            return {};
        }

        std::chrono::high_resolution_clock::time_point now =
            std::chrono::high_resolution_clock::now();

        if (!force && last_update_.has_value()) {
            std::chrono::duration<float> duration_s = now - last_update_.value();

            float frame_dur = 1 / static_cast<float>(fps_);
            float past_target = duration_s.count() - frame_dur + error_;
            if (past_target < 0) {
                return {};
            }

            error_ = past_target;
        }

        std::lock_guard guard(buffer_mutex_);

        std::optional<Frame> frame_opt = currentFrame();
        if (!frame_opt) {
            return {};
        }

        Frame frame = frame_opt.value();

        if (playback_ == Mirror) {
            if (frame.first >= last_frame_.load()) {
                reverse_ = true;
            } else if (frame.first == 0) {
                reverse_ = false;
            }
        }

        if (reverse_) {
            cursor_--;

            if (VIDREVOLT_VIDEO_MIDDLE  - cursor_ > WORK_THRESHOLD) {
                signalWork();
            }
        } else {
            cursor_++;

            if (cursor_ - VIDREVOLT_VIDEO_MIDDLE > WORK_THRESHOLD) {
                signalWork();
            }
        }

        last_update_ = std::chrono::high_resolution_clock::now();

        if (frame.first == last_frame_ && playback_ == Once) {
            finished_ = true;
        }

        return frame.second;
    }

    std::optional<Video::Frame> Video::currentFrame() {
        if (cursor_ < 0 || static_cast<size_t>(cursor_) >= buffer_.size()) {
            std::cerr << "WARNING: Video buffer exceeded! Try a a lower resolution video or increase key frames. Path:" <<
                path_ << std::endl;

            return {};
        } else {
            return buffer_.at(static_cast<size_t>(cursor_));
        }
    }

    Video::Frame Video::readFrame() {
        DEBUG_TIME_START(read_single)
        int pos = static_cast<int>(vid_->get(cv::CAP_PROP_POS_FRAMES));
        cv::Mat frame;
        if (vid_->read(frame)) {
            // TODO: Don't modify the matrix, simply load the opengl texture differently
            DEBUG_TIME_START(frame_processing)
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
            flip(frame, frame, 0);
            DEBUG_TIME_END(frame_processing)
        } else {
            vid_->set(cv::CAP_PROP_POS_FRAMES, 0);

            return readFrame();
        }

        DEBUG_TIME_END(read_single)

        return std::make_pair(pos, frame);
    }

    void Video::seek(int pos) {
        DEBUG_TIME_START(seek)
        if (pos < 0) {
            pos += total_frames_;
        }

        pos = pos % total_frames_;

        vid_->set(cv::CAP_PROP_POS_FRAMES, pos);
        DEBUG_TIME_END(seek)
    }

    double Video::getRemainingMS() {
        return length_ms - vid_->get(cv::CAP_PROP_POS_MSEC);
    }

    void Video::next() {
        // At the end of the day, this is where we want the read cursor to end up.
        int middle = VIDREVOLT_VIDEO_MIDDLE;

        // If we have a reset request, set the cursor to the start of the video
        // if it exists in our buffer.
        if (requested_reset_.load() && !buffer_.empty()) {
            std::lock_guard guard(buffer_mutex_);

            requested_reset_ = false;
            reverse_ = false;

            bool found = false;
            for (size_t i=0; i < VIDREVOLT_VIDEO_BUFFER_SIZE; i++) {
                if (buffer_.at(i).first == 0) {
                    found = true;
                    cursor_ = static_cast<int>(i);
                    break;
                }
            }

            if (!found) {
                buffer_.clear();
            }
        }

        if (buffer_.empty()) {
            std::lock_guard guard(buffer_mutex_);

            // Start from half the buffersize before the end of the video.
            seek(-middle);
            for (size_t i=0; i < VIDREVOLT_VIDEO_BUFFER_SIZE; i++) {
                Frame frame = readFrame();
                if (frame.first == 0) {
                    cursor_ = static_cast<int>(i);
                }

                buffer_.push_back(frame);
            }

            return;
        }

        // Calculate current distance from center.
        // Overshooting and undershooting are non-issues.
        int diff, front_pos, back_pos;
        {
            std::lock_guard guard(buffer_mutex_);

            diff = cursor_ - middle;;
            front_pos = buffer_.front().first;
            back_pos = buffer_.back().first;
        }

        // Fill in order to make the current frame the center frame of the buffer.
        if (diff < 0) {
            // Absolute diff.
            diff *= -1;

            // Jump back by the amount we need to read to catch up.
            seek(front_pos - diff);

            DEBUG_TIME_START(read_rev)
            std::vector<Frame> tmp_buf;
            for (int i=0; i < diff; i++) {
                tmp_buf.push_back(readFrame());
            }
            DEBUG_TIME_END(read_rev)

            {
                std::lock_guard guard(buffer_mutex_);
                buffer_.insert(buffer_.begin(), tmp_buf.begin(), tmp_buf.end());
                buffer_.erase(buffer_.end() - diff, buffer_.end());

                // Compensate for the current frame moving forwards.
                cursor_ += diff;
            }
        } else if (diff > 0) {
            int pos = static_cast<int>(vid_->get(cv::CAP_PROP_POS_FRAMES));
            if (pos != back_pos + 1) {
                seek(back_pos + 1);
            }

            std::vector<Frame> tmp_buf;
            for (int i=0; i < diff; i++) {
                tmp_buf.push_back(readFrame());
            }

            {
                std::lock_guard guard(buffer_mutex_);
                buffer_.insert(buffer_.end(), tmp_buf.begin(), tmp_buf.end());
                buffer_.erase(buffer_.begin(), buffer_.begin() + diff);

                // Compensate for the current frame moving backwards.
                cursor_ -= diff;
            }
        }
    }

    void Video::setFPS(double fps) {
        fps_ = fps;
    }

    double Video::getFPS() const {
        return fps_;
    }

    void Video::start() {
        if (running_.load()) {
            return;
        }

        vid_ = std::make_unique<cv::VideoCapture>(path_);

        if (!vid_->isOpened()) {
            throw std::runtime_error("Unable to open video with path " + path_);
        }

        // Explore metadata
        // Go to end of file
        vid_->set(cv::CAP_PROP_POS_AVI_RATIO, 1);
        total_frames_ = static_cast<int>(vid_->get(cv::CAP_PROP_POS_FRAMES));
        last_frame_ = total_frames_ - 1;
        length_ms = vid_->get(cv::CAP_PROP_POS_MSEC);
        vid_->set(cv::CAP_PROP_POS_AVI_RATIO, 0);

        // This is a guess, apparently it can be wrong
        total_frames_ = static_cast<int>(vid_->get(cv::CAP_PROP_FRAME_COUNT));
        if (total_frames_ < 0) {
            throw std::runtime_error("Unable to accurately determine number of frames for " + path_);
        }

        last_frame_ = total_frames_ - 1;

        fps_ = vid_->get(cv::CAP_PROP_FPS);
        if (fps_ <= 0) {
            throw std::runtime_error("Unable to accurately determine number FPS for " + path_);
        }

        running_ = true;
        load_mutex_.lock();
        thread_ = std::thread([this] {
            next();
            res_.width = buffer_.front().second.size().width;
            res_.height = buffer_.front().second.size().height;
            load_mutex_.unlock();

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

                next();

                {
                    std::lock_guard lk(work_ready_mutex_);
                    work_ready_ = false;
                }
            }
        });
    }

    Resolution Video::getResolution() {
        return res_;
    }

    void Video::flipPlayback() {
        setReverse(!reverse_);
    }

    void Video::waitForLoaded() {
        std::lock_guard<std::mutex> mut(load_mutex_);
    }

    void Video::setReverse(bool t) {
        reverse_ = t;
    }

    void Video::stop() {
        running_ = false;
        {
            std::lock_guard guard(work_ready_mutex_);
            work_ready_cv_.notify_one();
        }
        if (thread_.joinable()) {
            thread_.join();
        }
    }
}
