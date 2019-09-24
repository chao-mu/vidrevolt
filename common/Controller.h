#ifndef VIDREVOLT_CONTROLLER_H_
#define VIDREVOLT_CONTROLLER_H_

// STL
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <mutex>

// Boost
#include <boost/signals2.hpp>

// Ours
#include "Value.h"

namespace vidrevolt {
    class Controller {
        public:
            virtual ~Controller() = default;

            std::vector<std::string> getControlNames() const;

            virtual void beforePoll();

            void poll();
            void connect(const std::string& control_name, std::function<void(Value)> f);
            Value getValue(const std::string& control_name) const;

        protected:
            void addValue(const std::string& key, Value v);
            void addControlName(const std::string& key);
            void setControlNames(std::vector<std::string> control_names);

        private:
            std::vector<std::string> control_names_;
            std::map<std::string, boost::signals2::signal<void(Value)>> signals_;
            std::map<std::string, Value> values_;

            std::mutex pending_values_mutex_;
            std::vector<std::pair<std::string, Value>> pending_values_;
    };
}

#endif
