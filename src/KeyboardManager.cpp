#include "KeyboardManager.h"

namespace vidrevolt {
    const std::map<int, std::string> KeyboardManager::codes = KeyboardManager::makeCodeMappings();
    std::vector<std::shared_ptr<Keyboard>> KeyboardManager::keyboards_;

    std::map<int, std::string> KeyboardManager::makeCodeMappings() {
        std::map<int, std::string> codes;

        codes[GLFW_KEY_SPACE] = "space";
        codes[GLFW_KEY_APOSTROPHE] = "apostrophe";
        codes[GLFW_KEY_COMMA] = "comma";
        codes[GLFW_KEY_MINUS] = "minus";
        codes[GLFW_KEY_PERIOD] = "period";
        codes[GLFW_KEY_SLASH] = "slash";
        codes[GLFW_KEY_0] = "0";
        codes[GLFW_KEY_1] = "1";
        codes[GLFW_KEY_2] = "2";
        codes[GLFW_KEY_3] = "3";
        codes[GLFW_KEY_4] = "4";
        codes[GLFW_KEY_5] = "5";
        codes[GLFW_KEY_6] = "6";
        codes[GLFW_KEY_7] = "7";
        codes[GLFW_KEY_8] = "8";
        codes[GLFW_KEY_9] = "9";
        codes[GLFW_KEY_SEMICOLON] = "semicolon";
        codes[GLFW_KEY_EQUAL] = "equal";
        codes[GLFW_KEY_A] = "a";
        codes[GLFW_KEY_B] = "b";
        codes[GLFW_KEY_C] = "c";
        codes[GLFW_KEY_D] = "d";
        codes[GLFW_KEY_E] = "e";
        codes[GLFW_KEY_F] = "f";
        codes[GLFW_KEY_G] = "g";
        codes[GLFW_KEY_H] = "h";
        codes[GLFW_KEY_I] = "i";
        codes[GLFW_KEY_J] = "j";
        codes[GLFW_KEY_K] = "k";
        codes[GLFW_KEY_L] = "l";
        codes[GLFW_KEY_M] = "m";
        codes[GLFW_KEY_N] = "n";
        codes[GLFW_KEY_O] = "o";
        codes[GLFW_KEY_P] = "p";
        codes[GLFW_KEY_Q] = "q";
        codes[GLFW_KEY_R] = "r";
        codes[GLFW_KEY_S] = "s";
        codes[GLFW_KEY_T] = "t";
        codes[GLFW_KEY_U] = "u";
        codes[GLFW_KEY_V] = "v";
        codes[GLFW_KEY_W] = "w";
        codes[GLFW_KEY_X] = "x";
        codes[GLFW_KEY_Y] = "y";
        codes[GLFW_KEY_Z] = "z";
        codes[GLFW_KEY_LEFT_BRACKET] = "left_bracket";
        codes[GLFW_KEY_BACKSLASH] = "backslash";
        codes[GLFW_KEY_RIGHT_BRACKET] = "right_bracket";
        codes[GLFW_KEY_GRAVE_ACCENT] = "grave_accent";
        codes[GLFW_KEY_WORLD_1] = "world_1";
        codes[GLFW_KEY_WORLD_2] = "world_2";
        codes[GLFW_KEY_ESCAPE] = "escape";
        codes[GLFW_KEY_ENTER] = "enter";
        codes[GLFW_KEY_TAB] = "tab";
        codes[GLFW_KEY_BACKSPACE] = "backspace";
        codes[GLFW_KEY_INSERT] = "insert";
        codes[GLFW_KEY_DELETE] = "delete";
        codes[GLFW_KEY_RIGHT] = "right";
        codes[GLFW_KEY_LEFT] = "left";
        codes[GLFW_KEY_DOWN] = "down";
        codes[GLFW_KEY_UP] = "up";
        codes[GLFW_KEY_PAGE_UP] = "page_up";
        codes[GLFW_KEY_PAGE_DOWN] = "page_down";
        codes[GLFW_KEY_HOME] = "home";
        codes[GLFW_KEY_END] = "end";
        codes[GLFW_KEY_CAPS_LOCK] = "caps_lock";
        codes[GLFW_KEY_SCROLL_LOCK] = "scroll_lock";
        codes[GLFW_KEY_NUM_LOCK] = "num_lock";
        codes[GLFW_KEY_PRINT_SCREEN] = "print_screen";
        codes[GLFW_KEY_PAUSE] = "pause";
        codes[GLFW_KEY_F1] = "f1";
        codes[GLFW_KEY_F2] = "f2";
        codes[GLFW_KEY_F3] = "f3";
        codes[GLFW_KEY_F4] = "f4";
        codes[GLFW_KEY_F5] = "f5";
        codes[GLFW_KEY_F6] = "f6";
        codes[GLFW_KEY_F7] = "f7";
        codes[GLFW_KEY_F8] = "f8";
        codes[GLFW_KEY_F9] = "f9";
        codes[GLFW_KEY_F10] = "f10";
        codes[GLFW_KEY_F11] = "f11";
        codes[GLFW_KEY_F12] = "f12";
        codes[GLFW_KEY_F13] = "f13";
        codes[GLFW_KEY_F14] = "f14";
        codes[GLFW_KEY_F15] = "f15";
        codes[GLFW_KEY_F16] = "f16";
        codes[GLFW_KEY_F17] = "f17";
        codes[GLFW_KEY_F18] = "f18";
        codes[GLFW_KEY_F19] = "f19";
        codes[GLFW_KEY_F20] = "f20";
        codes[GLFW_KEY_F21] = "f21";
        codes[GLFW_KEY_F22] = "f22";
        codes[GLFW_KEY_F23] = "f23";
        codes[GLFW_KEY_F24] = "f24";
        codes[GLFW_KEY_F25] = "f25";
        codes[GLFW_KEY_KP_0] = "kp_0";
        codes[GLFW_KEY_KP_1] = "kp_1";
        codes[GLFW_KEY_KP_2] = "kp_2";
        codes[GLFW_KEY_KP_3] = "kp_3";
        codes[GLFW_KEY_KP_4] = "kp_4";
        codes[GLFW_KEY_KP_5] = "kp_5";
        codes[GLFW_KEY_KP_6] = "kp_6";
        codes[GLFW_KEY_KP_7] = "kp_7";
        codes[GLFW_KEY_KP_8] = "kp_8";
        codes[GLFW_KEY_KP_9] = "kp_9";
        codes[GLFW_KEY_KP_DECIMAL] = "kp_decimal";
        codes[GLFW_KEY_KP_DIVIDE] = "kp_divide";
        codes[GLFW_KEY_KP_MULTIPLY] = "kp_multiply";
        codes[GLFW_KEY_KP_SUBTRACT] = "kp_subtract";
        codes[GLFW_KEY_KP_ADD] = "kp_add";
        codes[GLFW_KEY_KP_ENTER] = "kp_enter";
        codes[GLFW_KEY_KP_EQUAL] = "kp_equal";
        codes[GLFW_KEY_LEFT_SHIFT] = "left_shift";
        codes[GLFW_KEY_LEFT_CONTROL] = "left_control";
        codes[GLFW_KEY_LEFT_ALT] = "left_alt";
        codes[GLFW_KEY_LEFT_SUPER] = "left_super";
        codes[GLFW_KEY_RIGHT_SHIFT] = "right_shift";
        codes[GLFW_KEY_RIGHT_CONTROL] = "right_control";
        codes[GLFW_KEY_RIGHT_ALT] = "right_alt";
        codes[GLFW_KEY_RIGHT_SUPER] = "right_super";
        codes[GLFW_KEY_MENU] = "menu";

        return codes;
    }

    void KeyboardManager::onKey(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/) {
        if (codes.count(key) > 0) {
            auto& name = codes.at(key);

            bool press;
            if (action == GLFW_PRESS) {
                press = true;
            } else if (action == GLFW_RELEASE) {
                press = false;
            } else {
                return;
            }

            for (auto& kb : keyboards_) {
                kb->onKey(name, press);
            }
        }
    }

    std::shared_ptr<Keyboard> KeyboardManager::makeKeyboard() {
        auto kb = std::make_shared<Keyboard>();
        keyboards_.push_back(kb);
        return kb;
    }
}
