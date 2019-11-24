#ifndef VIDREVOLT_KEYBOARDMANAGER_H_
#define VIDREVOLT_KEYBOARDMANAGER_H_

// STL
#include <map>
#include <memory>

// OpenGL
#include <GLFW/glfw3.h>

// Ours
#include "Keyboard.h"

namespace vidrevolt {
    class KeyboardManager {
        public:
            static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods);
            static std::shared_ptr<Keyboard> makeKeyboard();

        private:
            static std::map<int, std::string> makeCodeMappings();
            static const std::map<int, std::string> codes;
            static std::vector<std::shared_ptr<Keyboard>> keyboards_;
    };
}

#endif

