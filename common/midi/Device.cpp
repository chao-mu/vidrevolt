#include "Device.h"

// yaml-cpp
#include "yaml-cpp/yaml.h"

// Ours
#include "Message.h"
#include "../mathutil.h"

#define SLEEP_FOR_MS 5

namespace vidrevolt {
    namespace midi {
        Device::Device(const std::string& path) : path_(path) {
        }

        Device::~Device() {
            stop();
        }

        void Device::onError(RtMidiError::Type /*type*/, const std::string &msg, void* /*self*/) {
            std::cerr << "Midi error: " << msg << std::endl;
        }

        bool Device::connect() {
            unsigned int port_count = midi_in_->getPortCount();
            for (unsigned int i = 0; i < port_count; i++) {
                std::string name = midi_in_->getPortName(i);
                if (std::regex_search(name, name_re_)) {
                    midi_in_->openPort(i);
                    connected_ = true;
                    port_name_ = name;
                    return true;
                }
            }

            return false;
        }

        void Device::start() {
            if (running_.load()) {
                throw std::runtime_error("midi::Device already started");
            }

            midi_in_ = std::make_unique<RtMidiIn>();
            midi_in_->setErrorCallback(Device::onError, this);

            loadSettings();

            if (!connect()) {
                throw std::runtime_error("Requested midi device not found. Config loaded from " + path_);
            }

            running_ = true;
            thread_ = std::thread(&Device::loop, this);
        }

        void Device::loadSettings() {
            YAML::Node settings = YAML::LoadFile(path_);
            if (!settings["regex"]) {
                throw std::runtime_error("Expected field 'regex' to be found in " + path_);
            }

            name_re_ = std::regex(settings["regex"].as<std::string>());

            if (!settings["mappings"]) {
                throw std::runtime_error("Expected field 'mappings' to be found in " + path_);
            }

            for (const auto& mapping : settings["mappings"]) {
                std::string name = mapping.first.as<std::string>();
                YAML::Node props = mapping.second;

                Control control;
                std::vector<int> channels;
                if (props["channel"].IsSequence()) {
                    for (const auto& node : props["channel"]) {
                        channels.push_back(node.as<int>());
                    }
                } else {
                    channels.push_back(props["channel"].as<int>());
                }

                control.function = static_cast<unsigned char>(props["function"].as<int>());

                control.low = static_cast<unsigned char>(
                        props["low"] ? props["low"].as<int>() : 0);

                control.high = static_cast<unsigned char>(
                        props["high"] ? props["high"].as<int>() : 0);

                control.type = props["type"].as<std::string>() == "button" ? CONTROL_TYPE_BUTTON : CONTROL_TYPE_FADER;
                control.name = name;

                for (const auto& chan : channels) {
                    control.channel = static_cast<unsigned char>(chan);
                    controls_.push_back(control);
                }

                addControlName(name);
            }
        }

        void Device::stop() {
            if (thread_.joinable()) {
                running_ = false;
                thread_.join();
            }
        }

        void Device::loop() {
            while (running_.load()) {
                std::vector<unsigned char> raw_message;
                while (midi_in_->getMessage(&raw_message) > 0) {
                    Message msg(raw_message);

                    ControlType type = CONTROL_TYPE_UNKNOWN;
                    if (msg.getType() == MESSAGE_TYPE_NOTE_ON || msg.getType() == MESSAGE_TYPE_NOTE_OFF) {
                        type = CONTROL_TYPE_BUTTON;
                    } else if (msg.getType() == MESSAGE_TYPE_CONTROL) {
                        type = CONTROL_TYPE_FADER;
                    } else {
                        continue;
                    }

                    {
                        for (const auto& control : controls_) {
                            std::string name = control.name;
                            if (control.function == msg.getFunction() && control.type == type && msg.getChannel() == control.channel) {
                                float value = msg.getValue();
                                if (msg.getType() == MESSAGE_TYPE_NOTE_OFF) {
                                    value = 0;
                                }

                                value = mathutil::remap(value, control.low, control.high, 0, 1);

                                addValue(name, Value(value));
                            }
                        }
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_MS));
            }
        }
    }
}
