#include "Keyboard.h"

#include <iostream>

namespace vidrevolt {
    Keyboard::Keyboard() {
        control_names_ = {
            "space", "apostrophe", "comma", "minus", "period", "slash", "0", "1", "2", "3", "4",
            "5", "6", "7", "8", "9", "semicolon", "equal", "a", "b", "c", "d", "e", "f", "g", "h",
            "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y",
            "z", "left_bracket", "backslash", "right_bracket", "grave_accent", "world_1",
            "world_2", "escape", "enter", "tab", "backspace", "insert", "delete", "right", "left",
            "down", "up", "page_up", "page_down", "home", "end", "caps_lock", "scroll_lock",
            "num_lock", "print_screen", "pause", "f1", "f2", "f3", "f4", "f5", "f6", "f7",
            "f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15", "f16", "f17", "f18", "f19",
            "f20", "f21", "f22", "f23", "f24", "f25", "kp_0", "kp_1", "kp_2", "kp_3", "kp_4",
            "kp_5", "kp_6", "kp_7", "kp_8", "kp_9", "kp_decimal", "kp_divide", "kp_multiply",
            "kp_subtract", "kp_add", "kp_enter", "kp_equal", "left_shift", "left_control",
            "left_alt", "left_super", "right_shift", "right_control", "right_alt", "right_super",
            "menu"
        };
    }

    void Keyboard::onKey(const std::string& name, bool press) {
        addValue(name, Value(press));
    }

    std::vector<std::string> Keyboard::getControlNames() const {
        return control_names_;
    }
}
