#include "lunaris/editor/sidebar.h"
#include "lunaris/editor/file_tree.h"
#include "lunaris/core/theme.h"
#include "lunaris/ui/components.h"
#include <imgui.h>
#include <tinyvk/core/file_dialog.h>
#include <tinyvk/assets/icons_font_awesome.h>
#include <cstring>

namespace lunaris {

Sidebar::Sidebar()
    : _theme(nullptr)
    , _plugin_manager(nullptr)
    , _file_tree(nullptr)
    , _panel_visible(true)
    , _is_resizing(false)
    , _active_panel(SidebarPanel::Explorer)
    , _content_width(DEFAULT_CONTENT_WIDTH) {
    memset(_search_buffer, 0, sizeof(_search_buffer));
    _file_tree = new FileTree();
}

Sidebar::~Sidebar() {
    if (_file_tree) {
        delete _file_tree;
        _file_tree = nullptr;
    }
}

void Sidebar::set_theme(Theme* theme) {
    _theme = theme;
    if (_file_tree) {
        _file_tree->set_theme(theme);
    }
}

void Sidebar::on_init() {
}

void Sidebar::on_shutdown() {
}

void Sidebar::on_ui() {
    Color bg = _theme ? _theme->get_surface() : Color(0.11f, 0.11f, 0.13f);
    Color content_bg = _theme ? _theme->get_background() : Color(0.1f, 0.1f, 0.12f);

    ImVec2 sidebar_start = ImGui::GetCursorScreenPos();
    float total_h = ImGui::GetContentRegionAvail().y;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(bg.r, bg.g, bg.b, 1.0f));
    ImGui::BeginChild("##SidebarTabs", ImVec2(ICON_BAR_WIDTH, total_h), false, ImGuiWindowFlags_NoScrollbar);
    draw_panel_tabs();
    ImGui::EndChild();
    ImGui::PopStyleColor();

    if (_panel_visible) {
        ImGui::SameLine(0.0f, 0.0f);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(content_bg.r, content_bg.g, content_bg.b, 1.0f));
        ImGui::BeginChild("##SidebarContent", ImVec2(_content_width, total_h), false, ImGuiWindowFlags_NoScrollbar);
        ImGui::SetCursorPos(ImVec2(12.0f, 12.0f));
        ImGui::BeginGroup();
        switch (_active_panel) {
            case SidebarPanel::Explorer:
                draw_explorer();
                break;
            case SidebarPanel::Search:
                draw_search();
                break;
            default:
                ImGui::TextDisabled("Panel not implemented");
                break;
        }
        ImGui::EndGroup();
        ImGui::EndChild();
        ImGui::PopStyleColor();

        draw_resize_handle(sidebar_start.x, sidebar_start.y, total_h);
    }

    ImGui::PopStyleVar(2);
}

void Sidebar::draw_panel_tabs() {
    Color accent = _theme ? _theme->get_accent() : Color(0.3f, 0.5f, 0.8f);
    Color text = _theme ? _theme->get_text() : Color(0.9f, 0.9f, 0.92f);
    Color text_dim = _theme ? _theme->get_text_dim() : Color(0.5f, 0.5f, 0.52f);

    struct PanelDef {
        SidebarPanel panel;
        const char* icon;
        const char* tooltip;
    };

    PanelDef panels[] = {
        { SidebarPanel::Explorer, ICON_FA_FOLDER, "Explorer" },
        { SidebarPanel::Search, ICON_FA_MAGNIFYING_GLASS, "Search" },
        { SidebarPanel::SourceControl, ICON_FA_CODE_BRANCH, "Source Control" },
        { SidebarPanel::Debug, ICON_FA_BUG, "Debug" },
        { SidebarPanel::Extensions, ICON_FA_PUZZLE_PIECE, "Extensions" }
    };

    constexpr float BTN_SIZE = 24.0f;
    constexpr float PADDING = 8.0f;
    constexpr float SPACING = 6.0f;

    ImGui::SetCursorPos(ImVec2(PADDING, 12.0f));

    for (int i = 0; i < 5; ++i) {
        const auto& p = panels[i];
        bool is_active = (_active_panel == p.panel);

        ImVec4 txt = is_active
            ? ImVec4(text.r, text.g, text.b, 1.0f)
            : ImVec4(text_dim.r, text_dim.g, text_dim.b, 1.0f);

        ImGui::PushID(i);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(accent.r, accent.g, accent.b, 0.15f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(accent.r, accent.g, accent.b, 0.25f));
        ImGui::PushStyleColor(ImGuiCol_Text, txt);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

        if (is_active) {
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImVec2(pos.x - PADDING, pos.y + 2.0f),
                ImVec2(pos.x - PADDING + 2.0f, pos.y + BTN_SIZE - 2.0f),
                ImGui::ColorConvertFloat4ToU32(ImVec4(accent.r, accent.g, accent.b, 1.0f)),
                1.0f
            );
        }

        if (ImGui::Button(p.icon, ImVec2(BTN_SIZE, BTN_SIZE))) {
            _active_panel = p.panel;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ui::tooltip(p.tooltip);
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);
        ImGui::PopID();

        if (i < 4) {
            ImGui::SetCursorPosX(PADDING);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + SPACING);
        }
    }
}

void Sidebar::draw_explorer() {
    Color text = _theme ? _theme->get_text() : Color(0.9f, 0.9f, 0.92f);
    Color text_dim = _theme ? _theme->get_text_dim() : Color(0.5f, 0.5f, 0.52f);
    Color accent = _theme ? _theme->get_accent() : Color(0.3f, 0.5f, 0.8f);

    if (_file_tree && _file_tree->has_folder()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(text_dim.r, text_dim.g, text_dim.b, 1.0f));
        ImGui::TextUnformatted(_file_tree->get_root_name());
        ImGui::PopStyleColor();

        ImGui::Spacing();

        float avail_h = ImGui::GetContentRegionAvail().y;
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 2.0f));
        ImGui::BeginChild("##FileTree", ImVec2(-1, avail_h), false);
        _file_tree->on_ui();
        ImGui::EndChild();
        ImGui::PopStyleVar();
    } else {
        float avail_w = ImGui::GetContentRegionAvail().x;
        float avail_h = ImGui::GetContentRegionAvail().y;

        float content_h = 80.0f;
        float start_y = (avail_h - content_h) * 0.3f;
        if (start_y < 0) start_y = 0;

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + start_y);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(text_dim.r, text_dim.g, text_dim.b, 1.0f));
        float text_w = ImGui::CalcTextSize("No folder opened").x;
        ImGui::SetCursorPosX((avail_w - text_w) * 0.5f);
        ImGui::TextUnformatted("No folder opened");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Spacing();

        float btn_w = avail_w - 24.0f;
        if (btn_w < 80.0f) btn_w = 80.0f;
        if (btn_w > 140.0f) btn_w = 140.0f;

        ImGui::SetCursorPosX((avail_w - btn_w) * 0.5f);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(accent.r * 0.15f, accent.g * 0.15f, accent.b * 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(accent.r * 0.25f, accent.g * 0.25f, accent.b * 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(accent.r * 0.35f, accent.g * 0.35f, accent.b * 0.35f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(text.r, text.g, text.b, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 8.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

        if (ImGui::Button("Open Folder", ImVec2(btn_w, 0))) {
            open_folder();
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);
    }
}

void Sidebar::open_folder() {
    auto result = tvk::FileDialog::SelectFolder();
    if (result.has_value()) {
        open_folder(result->c_str());
    }
}

void Sidebar::open_folder(const char* path) {
    if (_file_tree && path) {
        _file_tree->open_folder(path);
    }
}

void Sidebar::close_folder() {
    if (_file_tree) {
        _file_tree->close_folder();
    }
}

bool Sidebar::has_folder() const {
    return _file_tree && _file_tree->has_folder();
}

const char* Sidebar::get_folder_path() const {
    return _file_tree ? _file_tree->get_root_path() : nullptr;
}

void Sidebar::set_file_selected_callback(FileSelectedCallback cb, void* user_data) {
    if (_file_tree) {
        _file_tree->set_file_selected_callback(cb, user_data);
    }
}

void Sidebar::draw_search() {
    Color text = _theme ? _theme->get_text() : Color(0.9f, 0.9f, 0.92f);
    Color text_dim = _theme ? _theme->get_text_dim() : Color(0.5f, 0.5f, 0.52f);
    Color bg_input = _theme ? _theme->get_background() : Color(0.08f, 0.08f, 0.1f);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(text_dim.r, text_dim.g, text_dim.b, 1.0f));
    ImGui::TextUnformatted("SEARCH");
    ImGui::PopStyleColor();

    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(bg_input.r, bg_input.g, bg_input.b, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##search_input", "Search...", _search_buffer, sizeof(_search_buffer));

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

void Sidebar::set_content_width(float w) {
    if (w < MIN_CONTENT_WIDTH) w = MIN_CONTENT_WIDTH;
    if (w > MAX_CONTENT_WIDTH) w = MAX_CONTENT_WIDTH;
    _content_width = w;
}

void Sidebar::draw_resize_handle(float sidebar_start_x, float sidebar_start_y, float total_height) {
    constexpr float HANDLE_WIDTH = 12.0f;
    constexpr float CAPSULE_WIDTH = 3.0f;
    constexpr float CAPSULE_HEIGHT = 32.0f;

    Color border = _theme ? _theme->get_border() : Color(0.2f, 0.2f, 0.22f);
    Color accent = _theme ? _theme->get_accent() : Color(0.3f, 0.5f, 0.8f);
    Color text_dim = _theme ? _theme->get_text_dim() : Color(0.5f, 0.5f, 0.52f);

    float line_x = sidebar_start_x + ICON_BAR_WIDTH + _content_width;
    float handle_min_x = line_x - HANDLE_WIDTH * 0.5f;
    float handle_max_x = line_x + HANDLE_WIDTH * 0.5f;
    float handle_min_y = sidebar_start_y;
    float handle_max_y = sidebar_start_y + total_height;

    ImVec2 mouse_pos = ImGui::GetMousePos();
    bool is_hovered = mouse_pos.x >= handle_min_x && mouse_pos.x <= handle_max_x
                   && mouse_pos.y >= handle_min_y && mouse_pos.y <= handle_max_y;

    if (is_hovered && ImGui::IsMouseClicked(0)) {
        _is_resizing = true;
    }

    if (_is_resizing) {
        if (ImGui::IsMouseDown(0)) {
            float new_width = mouse_pos.x - sidebar_start_x - ICON_BAR_WIDTH;
            set_content_width(new_width);
        } else {
            _is_resizing = false;
        }
    }

    if (is_hovered || _is_resizing) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    }

    ImDrawList* draw_list = ImGui::GetForegroundDrawList();

    ImU32 line_color = IM_COL32(
        (int)(border.r * 255),
        (int)(border.g * 255),
        (int)(border.b * 255),
        80
    );
    draw_list->AddLine(
        ImVec2(line_x, handle_min_y),
        ImVec2(line_x, handle_max_y),
        line_color, 1.0f
    );

    ImU32 capsule_color;
    if (_is_resizing) {
        capsule_color = IM_COL32(
            (int)(accent.r * 255),
            (int)(accent.g * 255),
            (int)(accent.b * 255),
            255
        );
    } else if (is_hovered) {
        capsule_color = IM_COL32(
            (int)(accent.r * 255),
            (int)(accent.g * 255),
            (int)(accent.b * 255),
            200
        );
    } else {
        capsule_color = IM_COL32(
            (int)(text_dim.r * 255),
            (int)(text_dim.g * 255),
            (int)(text_dim.b * 255),
            120
        );
    }

    float capsule_x = line_x - CAPSULE_WIDTH * 0.5f;
    float capsule_y = handle_min_y + (total_height - CAPSULE_HEIGHT) * 0.5f;
    draw_list->AddRectFilled(
        ImVec2(capsule_x, capsule_y),
        ImVec2(capsule_x + CAPSULE_WIDTH, capsule_y + CAPSULE_HEIGHT),
        capsule_color,
        CAPSULE_WIDTH * 0.5f
    );
}

}
