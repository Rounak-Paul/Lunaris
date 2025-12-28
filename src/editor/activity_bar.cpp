#include "lunaris/editor/activity_bar.h"
#include "lunaris/core/theme.h"
#include "lunaris/ui/components.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <tinyvk/assets/icons_font_awesome.h>
#include <cstring>

namespace lunaris {

ActivityBar::ActivityBar()
    : _activity_count(0)
    , _active_id(static_cast<ActivityID>(BuiltinActivity::Explorer))
    , _next_id(10)
    , _plugin_manager(nullptr)
    , _theme(nullptr) {
    for (uint32_t i = 0; i < MAX_ACTIVITIES; ++i) {
        _activities[i].id = INVALID_ACTIVITY_ID;
        _activities[i].info.name = nullptr;
        _activities[i].info.icon = nullptr;
        _activities[i].info.tooltip = nullptr;
        _activities[i].from_plugin = false;
    }
}

ActivityBar::~ActivityBar() {
}

void ActivityBar::on_init() {
    ActivityInfo explorer_info;
    explorer_info.name = "Explorer";
    explorer_info.icon = ICON_FA_FOLDER_OPEN;
    explorer_info.tooltip = "Explorer (Ctrl+Shift+E)";
    _activities[0].id = static_cast<ActivityID>(BuiltinActivity::Explorer);
    _activities[0].info = explorer_info;
    _activities[0].from_plugin = false;
    _activity_count++;

    ActivityInfo search_info;
    search_info.name = "Search";
    search_info.icon = ICON_FA_MAGNIFYING_GLASS;
    search_info.tooltip = "Search (Ctrl+Shift+F)";
    _activities[1].id = static_cast<ActivityID>(BuiltinActivity::Search);
    _activities[1].info = search_info;
    _activities[1].from_plugin = false;
    _activity_count++;

    ActivityInfo scm_info;
    scm_info.name = "Source Control";
    scm_info.icon = ICON_FA_CODE_BRANCH;
    scm_info.tooltip = "Source Control (Ctrl+Shift+G)";
    _activities[2].id = static_cast<ActivityID>(BuiltinActivity::SourceControl);
    _activities[2].info = scm_info;
    _activities[2].from_plugin = false;
    _activity_count++;

    ActivityInfo debug_info;
    debug_info.name = "Debug";
    debug_info.icon = ICON_FA_BUG;
    debug_info.tooltip = "Run and Debug (Ctrl+Shift+D)";
    _activities[3].id = static_cast<ActivityID>(BuiltinActivity::Debug);
    _activities[3].info = debug_info;
    _activities[3].from_plugin = false;
    _activity_count++;

    ActivityInfo ext_info;
    ext_info.name = "Extensions";
    ext_info.icon = ICON_FA_PUZZLE_PIECE;
    ext_info.tooltip = "Extensions (Ctrl+Shift+X)";
    _activities[4].id = static_cast<ActivityID>(BuiltinActivity::Extensions);
    _activities[4].info = ext_info;
    _activities[4].from_plugin = false;
    _activity_count++;
}

void ActivityBar::on_shutdown() {
}

ActivityID ActivityBar::generate_id() {
    return _next_id++;
}

ActivityID ActivityBar::register_activity(const ActivityInfo& info) {
    if (_activity_count >= MAX_ACTIVITIES) {
        return INVALID_ACTIVITY_ID;
    }

    for (uint32_t i = 0; i < MAX_ACTIVITIES; ++i) {
        if (_activities[i].id == INVALID_ACTIVITY_ID) {
            ActivityID id = generate_id();
            _activities[i].id = id;
            _activities[i].info = info;
            _activities[i].from_plugin = true;
            _activity_count++;
            return id;
        }
    }

    return INVALID_ACTIVITY_ID;
}

void ActivityBar::unregister_activity(ActivityID id) {
    for (uint32_t i = 0; i < MAX_ACTIVITIES; ++i) {
        if (_activities[i].id == id && _activities[i].from_plugin) {
            _activities[i].id = INVALID_ACTIVITY_ID;
            _activities[i].info.name = nullptr;
            _activities[i].info.icon = nullptr;
            _activities[i].info.tooltip = nullptr;
            _activity_count--;

            if (_active_id == id) {
                _active_id = static_cast<ActivityID>(BuiltinActivity::Explorer);
            }
            return;
        }
    }
}

void ActivityBar::set_active(ActivityID id) {
    for (uint32_t i = 0; i < MAX_ACTIVITIES; ++i) {
        if (_activities[i].id == id) {
            _active_id = id;
            return;
        }
    }
}

void ActivityBar::on_ui() {
    for (uint32_t i = 0; i < MAX_ACTIVITIES; ++i) {
        if (_activities[i].id != INVALID_ACTIVITY_ID) {
            draw_activity_button(i);
        }
    }

    float available_height = ui::get_content_height();
    float button_size = ICON_SIZE + 12.0f;

    if (available_height > button_size) {
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - button_size - 8.0f);
        float center_x = (BAR_WIDTH - button_size) * 0.5f;
        ImGui::SetCursorPosX(center_x);

        if (ui::icon_button(ICON_FA_GEAR, "settings_btn", false, button_size)) {
        }

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ui::tooltip("Settings", "Ctrl+,");
        }
    }
}

void ActivityBar::draw_activity_button(uint32_t index) {
    const Activity& activity = _activities[index];
    bool is_active = (_active_id == activity.id);

    float button_size = ICON_SIZE + 12.0f;
    float center_x = (BAR_WIDTH - button_size) * 0.5f;
    ImGui::SetCursorPosX(center_x);

    if (is_active) {
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ui::draw_active_indicator(button_size, 3.0f);
    }

    char id_buf[32];
    snprintf(id_buf, sizeof(id_buf), "activity_%u", activity.id);

    if (ui::icon_button(activity.info.icon, id_buf, is_active, button_size)) {
        _active_id = activity.id;
    }

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal) && activity.info.tooltip) {
        const char* shortcut = nullptr;
        switch (static_cast<BuiltinActivity>(activity.id)) {
            case BuiltinActivity::Explorer: shortcut = "Ctrl+Shift+E"; break;
            case BuiltinActivity::Search: shortcut = "Ctrl+Shift+F"; break;
            case BuiltinActivity::SourceControl: shortcut = "Ctrl+Shift+G"; break;
            case BuiltinActivity::Debug: shortcut = "Ctrl+Shift+D"; break;
            case BuiltinActivity::Extensions: shortcut = "Ctrl+Shift+X"; break;
            default: break;
        }
        ui::tooltip(activity.info.name, shortcut);
    }
}

}
