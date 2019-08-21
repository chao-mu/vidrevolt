#ifndef VIDREVOLT_TAPTEMPO_H_
#define VIDREVOLT_TAPTEMPO_H_

// STL
#include <vector>
#include <mutex>
#include <chrono>
#include <atomic>

// Boost
#include <boost/signals2.hpp>

// Ours
#include "Controller.h"

namespace vidrevolt {
    class BPMSync : public Controller {
        public:
            void tap();
            float predictNextBeat() const;
            float predictLastBeat() const;

            virtual void connect(const std::string& control_name, std::function<void(Value)> f) override;
            virtual std::vector<std::string> getControlNames() const override;

            virtual void tick() override;

        private:
            void reset();
            bool isTimedOut() const;
            double sinceLastTap() const;

            std::atomic<double> avg_distance_ = 0;

            std::vector<std::chrono::high_resolution_clock::time_point> taps_;
            mutable std::mutex taps_mutex_;

            std::chrono::high_resolution_clock::time_point last_tap_;
            mutable std::mutex last_tap_mutex_;

            boost::signals2::signal<void(Value)> next_beat_sig_;
            boost::signals2::signal<void(Value)> last_beat_sig_;
    };
}

#endif

