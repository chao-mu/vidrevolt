#include "BPMSync.h"

// STL
#include <cmath>
#include <iostream>

#define TAP_TEMPO_TIMEOUT_MS 2000

#define TAP_TEMPO_CTRL_NEXT_BEAT "next_beat"
#define TAP_TEMPO_CTRL_LAST_BEAT "last_beat"
#define TAP_TEMPO_CTRL_TRI "tri"

namespace vidrevolt {
    void BPMSync::tap() {
        std::chrono::high_resolution_clock::time_point now =
            std::chrono::high_resolution_clock::now();

        if (isTimedOut()) {
            reset();
        }

        double total_diff = 0;
        size_t diff_count = 0;

        {
            std::lock_guard<std::mutex> lck(taps_mutex_);

            taps_.push_back(now);

            for (size_t i = 0; i < taps_.size(); i++) {
                if (i + 1 < taps_.size()) {
                    total_diff +=
                        std::chrono::duration<double, std::milli>(taps_[i + 1] - taps_[i]).count();

                    diff_count += 1;
                }
            }
        }

        if (diff_count >= 1) {
            avg_distance_ = total_diff / static_cast<double>(diff_count);
        }

        last_tap_ = now;
    }

    float BPMSync::predictLastBeat() const {
        return 1 - predictNextBeat();
    }

    float BPMSync::predictTri() const {
        double since_last_tap = sinceLastTap();

        double whole;
        double fract = modf(since_last_tap / avg_distance_, &whole);
        float next = static_cast<float>(fract);

        if (static_cast<unsigned long>(whole) % 2 == 0) {
            return next;
        } else {
            return 1 - next;
        }
    }

    float BPMSync::predictNextBeat() const {
        double since_last_tap = sinceLastTap();

        double whole;
        double fract = modf(since_last_tap / avg_distance_, &whole);

        return static_cast<float>(fract);
    }

    double BPMSync::sinceLastTap() const {
        std::lock_guard<std::mutex> lck(last_tap_mutex_);

        return std::chrono::duration<double, std::milli>(
                std::chrono::high_resolution_clock::now() - last_tap_).count();
    }

    void BPMSync::reset() {
        std::lock_guard<std::mutex> lck(taps_mutex_);
        taps_.clear();
    }

    bool BPMSync::isTimedOut() const {
        double since_last_tap = sinceLastTap();

        // Have it appears we are no longer tapping to the same rythm
        return since_last_tap - avg_distance_ > TAP_TEMPO_TIMEOUT_MS;
    }

    void BPMSync::connect(const std::string& control_name, std::function<void(Value)> f) {
        if (control_name == TAP_TEMPO_CTRL_NEXT_BEAT) {
            next_beat_sig_.connect(f);
        } else if (control_name == TAP_TEMPO_CTRL_LAST_BEAT) {
            last_beat_sig_.connect(f);
        } else if (control_name == TAP_TEMPO_CTRL_TRI) {
            tri_sig_.connect(f);
        } else {
            throw std::runtime_error("BPMSync controller does not have control " + control_name);
        }
    }

    std::vector<std::string> BPMSync::getControlNames() const {
        return {TAP_TEMPO_CTRL_LAST_BEAT, TAP_TEMPO_CTRL_NEXT_BEAT, TAP_TEMPO_CTRL_TRI};
    }

    void BPMSync::tick() {
        next_beat_sig_(Value(predictNextBeat()));
        last_beat_sig_(Value(predictLastBeat()));
        tri_sig_(Value(predictLastBeat()));
    }
}
