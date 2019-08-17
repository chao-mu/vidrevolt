#ifndef FRAG_MIDI_DEVICE_H_
#define FRAG_MIDI_DEVICE_H_

// STL
#include <string>
#include <memory>
#include <mutex>
#include <map>
#include <thread>
#include <atomic>
#include <regex>
#include <vector>

// RtMidi
#include "rtmidi/RtMidi.h"

// Ours
#include "Control.h"

namespace frag {
    namespace midi {
        class Device {
            public:
                Device(const std::string& path);
                ~Device();

                void stop();
                void start();
                void update();

                void connect(const std::string& control_name, std::function<void(Value)> f);
                std::vector<std::string> getControlNames() const;

            private:
                void load();
                void loop();

                std::string path_;
                std::regex name_re_;
                mutable std::mutex controls_mutex_;
                std::string port_name_;

                std::shared_ptr<RtMidiIn> midi_in_;
                std::map<std::string, std::shared_ptr<Control>> controls_;
                std::thread thread_;
                std::atomic<bool> running_;
        };
    }
}
#endif
