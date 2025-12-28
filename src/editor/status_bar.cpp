#include "lunaris/editor/status_bar.h"
#include "lunaris/plugin/plugin_manager.h"
#include <imgui.h>
#include <cstring>

namespace lunaris {

StatusBar::StatusBar()
    : _plugin_manager(nullptr)
    , _progress(0.0f)
    , _show_progress(false) {
    _status_text[0] = '\0';
    _repo_name[0] = '\0';
    _branch_name[0] = '\0';
    _build_status[0] = '\0';
    _progress_label[0] = '\0';

    strncpy(_status_text, "Ready", sizeof(_status_text) - 1);
}

StatusBar::~StatusBar() {
}

void StatusBar::on_init() {
}

void StatusBar::on_shutdown() {
}

void StatusBar::set_status_text(const char* text) {
    if (text) {
        strncpy(_status_text, text, sizeof(_status_text) - 1);
        _status_text[sizeof(_status_text) - 1] = '\0';
    }
}

void StatusBar::set_repo_name(const char* name) {
    if (name) {
        strncpy(_repo_name, name, sizeof(_repo_name) - 1);
        _repo_name[sizeof(_repo_name) - 1] = '\0';
    }
}

void StatusBar::set_branch_name(const char* name) {
    if (name) {
        strncpy(_branch_name, name, sizeof(_branch_name) - 1);
        _branch_name[sizeof(_branch_name) - 1] = '\0';
    }
}

void StatusBar::set_build_status(const char* status) {
    if (status) {
        strncpy(_build_status, status, sizeof(_build_status) - 1);
        _build_status[sizeof(_build_status) - 1] = '\0';
    }
}

void StatusBar::set_progress(float progress, const char* label) {
    _progress = progress;
    _show_progress = true;
    if (label) {
        strncpy(_progress_label, label, sizeof(_progress_label) - 1);
        _progress_label[sizeof(_progress_label) - 1] = '\0';
    } else {
        _progress_label[0] = '\0';
    }
}

void StatusBar::clear_progress() {
    _progress = 0.0f;
    _show_progress = false;
    _progress_label[0] = '\0';
}

void StatusBar::on_ui() {
    float available_width = ImGui::GetContentRegionAvail().x;
    float section_width = available_width / 3.0f;

    ImGui::BeginGroup();
    draw_left_section();
    ImGui::EndGroup();

    ImGui::SameLine(section_width);

    ImGui::BeginGroup();
    draw_center_section();
    ImGui::EndGroup();

    ImGui::SameLine(section_width * 2.0f);

    ImGui::BeginGroup();
    draw_right_section();
    ImGui::EndGroup();

    if (_plugin_manager) {
        _plugin_manager->status_bar_all();
    }
}

void StatusBar::draw_left_section() {
    if (_repo_name[0] != '\0') {
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "%s", _repo_name);
        if (_branch_name[0] != '\0') {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.8f, 0.6f, 1.0f, 1.0f), "(%s)", _branch_name);
        }
    }
}

void StatusBar::draw_center_section() {
    if (_show_progress) {
        float progress_width = 200.0f;
        if (_progress_label[0] != '\0') {
            ImGui::Text("%s", _progress_label);
            ImGui::SameLine();
        }
        ImGui::ProgressBar(_progress, ImVec2(progress_width, 0.0f));
    } else if (_build_status[0] != '\0') {
        ImGui::Text("%s", _build_status);
    }
}

void StatusBar::draw_right_section() {
    ImGui::Text("%s", _status_text);
}

}
