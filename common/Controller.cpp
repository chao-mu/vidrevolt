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

    std::map<std::string, Value> Controller::getValues() const {
        return values_;
    }

    void Controller::reconnect() {
    }

    void Controller::connect(std::function<void(const std::string& name, Value)> f) {
        generic_signal_.connect(f);
    }

    void Controller::poll() {
        beforePoll();

        {
            std::lock_guard<std::mutex> lck(pending_values_mutex_);
            for (const auto& kv : pending_values_) {
                values_[kv.first] = kv.second;
                signals_[kv.first](kv.second);
                generic_signal_(kv.first, kv.second);
            }
            pending_values_.clear();
        }
    }

    void Controller::addControlName(const std::string& control_name) {
        control_names_.push_back(control_name);
        values_[control_name] = Value(0);
    }

    void Controller::setControlNames(std::vector<std::string> control_names) {
        control_names_ = control_names;
        for (const auto& name : control_names) {
            values_[name] = Value(0);
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

    std::vector<std::string> Controller::getControlNames() const {
        return control_names_;
    }
}
