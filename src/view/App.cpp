#include "App.h"

namespace ed = ax::NodeEditor;

namespace vidrevolt::view {
    App::App(std::shared_ptr<Pipeline> pipeline) : pipeline_(std::move(pipeline)) {
    }

    App::~App() {
        ed::DestroyEditor(ed_context_);
    }

    void App::setup() {
        ed::Config config;
        ed_context_ = ed::CreateEditor(&config);
    }

    void App::newFrame() {
        auto& io = ImGui::GetIO();

        ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

        ImGui::Separator();

        ed::SetCurrentEditor(ed_context_);
        ed::Begin("My Editor", ImVec2(0.0, 0.0f));
        int uniqueId = 1;
        // Start drawing nodes.
        ed::BeginNode(uniqueId++);
            ImGui::Text("Node A");
            ed::BeginPin(uniqueId++, ed::PinKind::Input);
                ImGui::Text("-> In");
            ed::EndPin();
            ImGui::SameLine();
            ed::BeginPin(uniqueId++, ed::PinKind::Output);
                ImGui::Text("Out ->");
            ed::EndPin();
        ed::EndNode();
        ed::End();
        ed::SetCurrentEditor(nullptr);

    }
}
