#include "Device.h"

// yaml-cpp
#include "yaml-cpp/yaml.h"

// Ours
#include "Message.h"
#include "../MathUtil.h"

#define SLEEP_FOR_MS 5

namespace frag {
    namespace midi {
        Device::Device(const std::string& path) : path_(path), midi_in_(new RtMidiIn()), running_(false) {
        }

        Device::~Device() {
            stop();
        }

        void Device::start() {
            if (running_.load()) {
                return;
            }

            load();
            unsigned int port_count = midi_in_->getPortCount();
            bool found = false;
            for (unsigned int i = 0; i < port_count; i++) {
                std::string name = midi_in_->getPortName(i);
                if (std::regex_search(name, name_re_)) {
                    midi_in_->openPort(i);
                    found = true;
                    break;
                }
            }

            if (!found) {
                throw std::runtime_error("Requested midi device not found. Config loaded from " + path_);
            }

            running_ = true;
            thread_ = std::thread(&Device::loop, this);
        }

        void Device::load() {
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

                std::shared_ptr<Control> control = std::make_shared<Control>();
                control->channel = (unsigned char) props["channel"].as<int>();
                control->function = (unsigned char) props["function"].as<int>();
                control->low = (unsigned char) props["low"].as<int>();
                control->high = (unsigned char) props["high"].as<int>();
                control->type = props["type"].as<std::string>() == "button" ? CONTROL_TYPE_BUTTON : CONTROL_TYPE_FADER;
                control->name = name;

                controls_[name] = control;
            }
        }

        void Device::stop() {
            if (thread_.joinable()) {
                running_ = false;
                thread_.join();
            }
        }

        std::vector<std::string> Device::getControlNames() const {
            std::vector<std::string> names;
            {
                std::lock_guard<std::mutex> guard(controls_mutex_);
                for (const auto& kv : controls_) {
                    names.push_back(kv.first);
                }
            }

            return names;
        }

        void Device::connect(const std::string& control_name, std::function<void(Value)> f) {
            if (controls_.count(control_name) > 0) {
                controls_.at(control_name)->change.connect(f);
            } else {
                throw std::runtime_error(
                        "Unknown control name " + control_name + " for midi device " + path_);
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
                        std::lock_guard<std::mutex> guard(controls_mutex_);
                        for (auto& kv : controls_) {
                            auto control = kv.second;
                            if (control->function == msg.getFunction() && control->type == type && msg.getChannel() == control->channel) {
                                float value = msg.getValue();
                                if (msg.getType() == MESSAGE_TYPE_NOTE_OFF) {
                                    value = 0;
                                }

                                if (control->type != CONTROL_TYPE_BUTTON) {
                                    value = remap(value, control->low, control->high, 0, 1);
                                }

                                control->change(Value(value));
                            }
                        }
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_MS));
            }
        }
    }
}
