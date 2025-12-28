#include "lunaris/editor/sidebar.h"
#include "lunaris/core/theme.h"
#include "lunaris/ui/components.h"
#include <imgui.h>
#include <tinyvk/assets/icons_font_awesome.h>
#include <cstring>

namespace lunaris {

Sidebar::Sidebar()
    : _theme(nullptr)
    , _plugin_manager(nullptr)
    , _panel_visible(true)
    , _active_panel(SidebarPanel::Explorer) {
    memset(_search_buffer, 0, sizeof(_search_buffer));
}

Sidebar::~Sidebar() {
}

void Sidebar::on_init() {
}

void Sidebar::on_shutdown() {
}

void Sidebar::on_ui() {
    Color bg = _theme ? _theme->get_surface() : Color(0.11f, 0.11f, 0.13f);
    Color content_bg = _theme ? _theme->get_background() : Color(0.1f, 0.1f, 0.12f);

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
        ImGui::BeginChild("##SidebarContent", ImVec2(CONTENT_WIDTH, total_h), false, ImGuiWindowFlags_NoScrollbar);
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

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(text_dim.r, text_dim.g, text_dim.b, 1.0f));
    ImGui::TextUnformatted("EXPLORER");
    ImGui::PopStyleColor();

    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(text.r, text.g, text.b, 1.0f));
    ImGui::TextDisabled("No folder opened");
    ImGui::Spacing();
    ImGui::TextWrapped("Open a folder to start working on a project.");
    ImGui::PopStyleColor();
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

}
