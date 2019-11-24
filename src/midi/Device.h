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
                virtual void reconnect() override;

            private:
                void loadSettings();
                void loop();
                bool connectDevice();

                static void onError(RtMidiError::Type type, const std::string &msg, void* self);

                std::string path_;
                std::regex name_re_;

                std::unique_ptr<RtMidiIn> midi_in_;
                std::vector<Control> controls_;
                std::thread thread_;
                std::atomic<bool> running_ = false;
                std::atomic<bool> reconnect_request_ = false;
        };
    }
}
#endif
