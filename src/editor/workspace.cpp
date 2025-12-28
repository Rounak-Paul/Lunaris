#include "lunaris/editor/workspace.h"
#include "lunaris/plugin/plugin_manager.h"
#include <imgui.h>

namespace lunaris {

Workspace::Workspace()
    : _plugin_manager(nullptr) {
}

Workspace::~Workspace() {
}

void Workspace::on_init() {
}

void Workspace::on_shutdown() {
}

void Workspace::on_update(float delta_time) {
    (void)delta_time;
}

void Workspace::on_ui() {
    ImGui::SetCursorPos(ImVec2(16.0f, 16.0f));
    ImGui::BeginGroup();

    if (_plugin_manager && _plugin_manager->get_plugin_count() > 0) {
        _plugin_manager->workspace_ui_all();
    } else {
        ImGui::Text("Lunaris Code Editor");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Open a file to start editing");
    }

    ImGui::EndGroup();
}

}
