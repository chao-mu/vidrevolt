#include "Controller.h"

#include <iostream>

namespace vidrevolt {
    void Controller::connect(const std::string& control_name, std::function<void(Value)> f) {
        auto names = getControlNames();
        if (std::find(names.begin(), names.end(), control_name) == names.end()) {
            throw std::runtime_error("control name '" + control_name + "' does not exist.");
        }

        signals_[control_name].connect(f);
    }

    void Controller::poll() {
        beforePoll();

        {
            std::lock_guard<std::mutex> lck(pending_values_mutex_);
            for (const auto& kv : pending_values_) {
                values_[kv.first] = kv.second;
                signals_[kv.first](kv.second);
            }
            pending_values_.clear();
        }
    }

    void Controller::addValue(const std::string& control_name, Value v) {
        std::lock_guard<std::mutex> lck(pending_values_mutex_);
        pending_values_.push_back(std::make_pair(control_name, v));
    }

    Value Controller::getValue(const std::string& control_name) const {
        if (values_.count(control_name) > 0) {
            return values_.at(control_name);
        }

        return Value();
    }

    void Controller::beforePoll() {
    }
}

