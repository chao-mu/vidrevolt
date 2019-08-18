#ifndef FRAG_MIDI_CONTROL_H_
#define FRAG_MIDI_CONTROL_H_

// STL
#include <string>
#include <functional>

// Boost
#include <boost/signals2.hpp>

// Ours
#include "../Value.h"

namespace vidrevolt {
    namespace midi {
        enum ControlType {
            CONTROL_TYPE_BUTTON,
            CONTROL_TYPE_FADER,
            CONTROL_TYPE_UNKNOWN
        };

        struct Control {
            bool isPressed();

            boost::signals2::signal<void(Value)> change;

            std::string name = "";
            ControlType type = CONTROL_TYPE_UNKNOWN;
            bool toggle = false;
            unsigned char low = 0;
            unsigned char high = 0;
            unsigned char function = 0;
            unsigned char channel = 0;
        };
    }
}
#endif
