#include "lunaris/editor/editor_layer.h"
#include <imgui.h>

namespace lunaris {

EditorLayer::EditorLayer()
    : _show_demo_window(false) {
}

EditorLayer::~EditorLayer() {
}

void EditorLayer::on_init() {
}

void EditorLayer::on_shutdown() {
}

void EditorLayer::on_update(float delta_time) {
    (void)delta_time;
}

void EditorLayer::on_ui() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
            }
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "Ctrl+X")) {
            }
            if (ImGui::MenuItem("Copy", "Ctrl+C")) {
            }
            if (ImGui::MenuItem("Paste", "Ctrl+V")) {
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Demo Window", nullptr, &_show_demo_window);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (_show_demo_window) {
        ImGui::ShowDemoWindow(&_show_demo_window);
    }

    ImGui::Begin("Welcome");
    ImGui::Text("Lunaris Code Editor");
    ImGui::Separator();
    ImGui::TextWrapped("A modern code editor built with TinyVK and ImGui.");
    ImGui::End();
}

}
