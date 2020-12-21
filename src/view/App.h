#ifndef VIDREVOLT_VIEW_APP_H_
#define VIDREVOLT_VIEW_APP_H_

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// ImGui Node Editor
#include <imgui_node_editor.h>

// OpenGL
#include "glad/glad.h"
#include <GLFW/glfw3.h>

// Ours
#include "Pipeline.h"

namespace vidrevolt::view {
    class App {
        public:
            App(std::shared_ptr<Pipeline> pipeline);
            ~App();

            void setup();
            void newFrame();

        private:
            ImVec4 clear_color_ = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
            std::shared_ptr<Pipeline> pipeline_;
            ax::NodeEditor::EditorContext* ed_context_ = nullptr;
    };
}
#endif
