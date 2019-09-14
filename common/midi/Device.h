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

// Ours (midi)
#include "Control.h"

// Ours
#include "../Controller.h"

namespace vidrevolt {
    namespace midi {
        class Device : public Controller {
            public:
                Device(const std::string& path);
                ~Device();

                void stop();
                void start();
                void update();

                virtual std::vector<std::string> getControlNames() const override;

            private:
                void loadSettings();
                bool connect();
                void loop();

                static void onError(RtMidiError::Type type, const std::string &msg, void* self);

                std::string path_;
                std::regex name_re_;
                mutable std::mutex controls_mutex_;
                std::string port_name_;

                std::unique_ptr<RtMidiIn> midi_in_;
                std::vector<Control> controls_;
                std::thread thread_;
                std::atomic<bool> running_ = false;
                std::vector<std::string> control_names_;
                std::atomic<bool> connected_ = false;
        };
    }
}
#endif
