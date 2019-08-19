#include "Keyboard.h"

namespace vidrevolt {
    std::map<int, boost::signals2::signal<void(Value)>> Keyboard::signals_;

    Keyboard::Keyboard() {
        codes_["space"] = GLFW_KEY_SPACE;
        codes_["apostrophe"] = GLFW_KEY_APOSTROPHE;
        codes_["comma"] = GLFW_KEY_COMMA;
        codes_["minus"] = GLFW_KEY_MINUS;
        codes_["period"] = GLFW_KEY_PERIOD;
        codes_["slash"] = GLFW_KEY_SLASH;
        codes_["0"] = GLFW_KEY_0;
        codes_["1"] = GLFW_KEY_1;
        codes_["2"] = GLFW_KEY_2;
        codes_["3"] = GLFW_KEY_3;
        codes_["4"] = GLFW_KEY_4;
        codes_["5"] = GLFW_KEY_5;
        codes_["6"] = GLFW_KEY_6;
        codes_["7"] = GLFW_KEY_7;
        codes_["8"] = GLFW_KEY_8;
        codes_["9"] = GLFW_KEY_9;
        codes_["semicolon"] = GLFW_KEY_SEMICOLON;
        codes_["equal"] = GLFW_KEY_EQUAL;
        codes_["a"] = GLFW_KEY_A;
        codes_["b"] = GLFW_KEY_B;
        codes_["c"] = GLFW_KEY_C;
        codes_["d"] = GLFW_KEY_D;
        codes_["e"] = GLFW_KEY_E;
        codes_["f"] = GLFW_KEY_F;
        codes_["g"] = GLFW_KEY_G;
        codes_["h"] = GLFW_KEY_H;
        codes_["i"] = GLFW_KEY_I;
        codes_["j"] = GLFW_KEY_J;
        codes_["k"] = GLFW_KEY_K;
        codes_["l"] = GLFW_KEY_L;
        codes_["m"] = GLFW_KEY_M;
        codes_["n"] = GLFW_KEY_N;
        codes_["o"] = GLFW_KEY_O;
        codes_["p"] = GLFW_KEY_P;
        codes_["q"] = GLFW_KEY_Q;
        codes_["r"] = GLFW_KEY_R;
        codes_["s"] = GLFW_KEY_S;
        codes_["t"] = GLFW_KEY_T;
        codes_["u"] = GLFW_KEY_U;
        codes_["v"] = GLFW_KEY_V;
        codes_["w"] = GLFW_KEY_W;
        codes_["x"] = GLFW_KEY_X;
        codes_["y"] = GLFW_KEY_Y;
        codes_["z"] = GLFW_KEY_Z;
        codes_["left_bracket"] = GLFW_KEY_LEFT_BRACKET;
        codes_["backslash"] = GLFW_KEY_BACKSLASH;
        codes_["right_bracket"] = GLFW_KEY_RIGHT_BRACKET;
        codes_["grave_accent"] = GLFW_KEY_GRAVE_ACCENT;
        codes_["world_1"] = GLFW_KEY_WORLD_1;
        codes_["world_2"] = GLFW_KEY_WORLD_2;
        codes_["escape"] = GLFW_KEY_ESCAPE;
        codes_["enter"] = GLFW_KEY_ENTER;
        codes_["tab"] = GLFW_KEY_TAB;
        codes_["backspace"] = GLFW_KEY_BACKSPACE;
        codes_["insert"] = GLFW_KEY_INSERT;
        codes_["delete"] = GLFW_KEY_DELETE;
        codes_["right"] = GLFW_KEY_RIGHT;
        codes_["left"] = GLFW_KEY_LEFT;
        codes_["down"] = GLFW_KEY_DOWN;
        codes_["up"] = GLFW_KEY_UP;
        codes_["page_up"] = GLFW_KEY_PAGE_UP;
        codes_["page_down"] = GLFW_KEY_PAGE_DOWN;
        codes_["home"] = GLFW_KEY_HOME;
        codes_["end"] = GLFW_KEY_END;
        codes_["caps_lock"] = GLFW_KEY_CAPS_LOCK;
        codes_["scroll_lock"] = GLFW_KEY_SCROLL_LOCK;
        codes_["num_lock"] = GLFW_KEY_NUM_LOCK;
        codes_["print_screen"] = GLFW_KEY_PRINT_SCREEN;
        codes_["pause"] = GLFW_KEY_PAUSE;
        codes_["f1"] = GLFW_KEY_F1;
        codes_["f2"] = GLFW_KEY_F2;
        codes_["f3"] = GLFW_KEY_F3;
        codes_["f4"] = GLFW_KEY_F4;
        codes_["f5"] = GLFW_KEY_F5;
        codes_["f6"] = GLFW_KEY_F6;
        codes_["f7"] = GLFW_KEY_F7;
        codes_["f8"] = GLFW_KEY_F8;
        codes_["f9"] = GLFW_KEY_F9;
        codes_["f10"] = GLFW_KEY_F10;
        codes_["f11"] = GLFW_KEY_F11;
        codes_["f12"] = GLFW_KEY_F12;
        codes_["f13"] = GLFW_KEY_F13;
        codes_["f14"] = GLFW_KEY_F14;
        codes_["f15"] = GLFW_KEY_F15;
        codes_["f16"] = GLFW_KEY_F16;
        codes_["f17"] = GLFW_KEY_F17;
        codes_["f18"] = GLFW_KEY_F18;
        codes_["f19"] = GLFW_KEY_F19;
        codes_["f20"] = GLFW_KEY_F20;
        codes_["f21"] = GLFW_KEY_F21;
        codes_["f22"] = GLFW_KEY_F22;
        codes_["f23"] = GLFW_KEY_F23;
        codes_["f24"] = GLFW_KEY_F24;
        codes_["f25"] = GLFW_KEY_F25;
        codes_["kp_0"] = GLFW_KEY_KP_0;
        codes_["kp_1"] = GLFW_KEY_KP_1;
        codes_["kp_2"] = GLFW_KEY_KP_2;
        codes_["kp_3"] = GLFW_KEY_KP_3;
        codes_["kp_4"] = GLFW_KEY_KP_4;
        codes_["kp_5"] = GLFW_KEY_KP_5;
        codes_["kp_6"] = GLFW_KEY_KP_6;
        codes_["kp_7"] = GLFW_KEY_KP_7;
        codes_["kp_8"] = GLFW_KEY_KP_8;
        codes_["kp_9"] = GLFW_KEY_KP_9;
        codes_["kp_decimal"] = GLFW_KEY_KP_DECIMAL;
        codes_["kp_divide"] = GLFW_KEY_KP_DIVIDE;
        codes_["kp_multiply"] = GLFW_KEY_KP_MULTIPLY;
        codes_["kp_subtract"] = GLFW_KEY_KP_SUBTRACT;
        codes_["kp_add"] = GLFW_KEY_KP_ADD;
        codes_["kp_enter"] = GLFW_KEY_KP_ENTER;
        codes_["kp_equal"] = GLFW_KEY_KP_EQUAL;
        codes_["left_shift"] = GLFW_KEY_LEFT_SHIFT;
        codes_["left_control"] = GLFW_KEY_LEFT_CONTROL;
        codes_["left_alt"] = GLFW_KEY_LEFT_ALT;
        codes_["left_super"] = GLFW_KEY_LEFT_SUPER;
        codes_["right_shift"] = GLFW_KEY_RIGHT_SHIFT;
        codes_["right_control"] = GLFW_KEY_RIGHT_CONTROL;
        codes_["right_alt"] = GLFW_KEY_RIGHT_ALT;
        codes_["right_super"] = GLFW_KEY_RIGHT_SUPER;
        codes_["menu"] = GLFW_KEY_MENU;

        for (const auto& kv : codes_) {
            control_names_.push_back(kv.first);
        }
    }

    void Keyboard::connect(const std::string& control_name, std::function<void(Value)> f) {
        if (!codes_.count(control_name)) {
            throw std::runtime_error("Keyboard control name '" + control_name + "' does not exist");
        }

        Keyboard::signals_[codes_.at(control_name)].connect(f);
    }

    void Keyboard::onKey(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/) {
        if (action == GLFW_PRESS) {
            Keyboard::signals_[key](Value(true));
        } else if (action == GLFW_RELEASE) {
            Keyboard::signals_[key](Value(false));
        }
    }

    std::vector<std::string> Keyboard::getControlNames() const {
        return control_names_;
    }
}
