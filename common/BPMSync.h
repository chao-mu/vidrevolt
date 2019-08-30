#ifndef VIDREVOLT_TAPTEMPO_H_
#define VIDREVOLT_TAPTEMPO_H_

// STL
#include <vector>
#include <mutex>
#include <chrono>
#include <atomic>

// Ours
#include "Controller.h"

namespace vidrevolt {
    class BPMSync : public Controller {
        public:
            void tap();
            float predictNextBeat() const;
            float predictLastBeat() const;
            float predictTri() const;

            virtual std::vector<std::string> getControlNames() const override;

            virtual void beforePoll() override;

        private:
            void reset();
            bool isTimedOut() const;
            double sinceLastTap() const;

            std::atomic<double> avg_distance_ = 0;

            std::vector<std::chrono::high_resolution_clock::time_point> taps_;
            mutable std::mutex taps_mutex_;

            std::chrono::high_resolution_clock::time_point last_tap_;
            mutable std::mutex last_tap_mutex_;
    };
}

#endif

