#include "lunaris/editor/bottom_panel.h"
#include "lunaris/core/theme.h"
#include "lunaris/ui/components.h"
#include <imgui.h>

namespace lunaris {

BottomPanel::BottomPanel()
    : _theme(nullptr)
    , _visible(false)
    , _height(HEIGHT)
    , _active_tab(BottomPanelTab::Terminal) {
}

BottomPanel::~BottomPanel() {
}

void BottomPanel::on_init() {
}

void BottomPanel::on_shutdown() {
}

void BottomPanel::on_ui() {
    if (!_visible) {
        return;
    }

    Color bg = _theme ? _theme->get_background() : Color(0.1f, 0.1f, 0.12f);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    draw_tabs();

    float content_h = _height - TAB_HEIGHT;
    float font_size = ImGui::GetFontSize();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(bg.r, bg.g, bg.b, 1.0f));
    ImGui::BeginChild("##PanelContent", ImVec2(0.0f, content_h), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::SetCursorPos(ImVec2(font_size * 0.75f, font_size * 0.5f));
    switch (_active_tab) {
        case BottomPanelTab::Terminal:
            draw_terminal();
            break;
        case BottomPanelTab::Output:
            draw_output();
            break;
        case BottomPanelTab::Problems:
            draw_problems();
            break;
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::PopStyleVar();
}

void BottomPanel::draw_tabs() {
    Color bg = _theme ? _theme->get_surface() : Color(0.12f, 0.12f, 0.14f);
    Color accent = _theme ? _theme->get_accent() : Color(0.3f, 0.5f, 0.8f);
    Color text = _theme ? _theme->get_text() : Color(0.9f, 0.9f, 0.92f);
    Color text_dim = _theme ? _theme->get_text_dim() : Color(0.5f, 0.5f, 0.52f);

    struct TabDef {
        BottomPanelTab tab;
        const char* label;
    };

    TabDef tabs[] = {
        { BottomPanelTab::Terminal, "Terminal" },
        { BottomPanelTab::Output, "Output" },
        { BottomPanelTab::Problems, "Problems" }
    };

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(bg.r, bg.g, bg.b, 1.0f));
    ImGui::BeginChild("##PanelTabs", ImVec2(0.0f, TAB_HEIGHT), false, ImGuiWindowFlags_NoScrollbar);

    float font_size_tabs = ImGui::GetFontSize();
    ImGui::SetCursorPos(ImVec2(font_size_tabs * 0.5f, 0.0f));

    for (int i = 0; i < 3; ++i) {
        if (i > 0) {
            ImGui::SameLine(0.0f, 0.0f);
        }

        const auto& t = tabs[i];
        bool is_active = (_active_tab == t.tab);

        ImVec4 txt = is_active
            ? ImVec4(text.r, text.g, text.b, 1.0f)
            : ImVec4(text_dim.r, text_dim.g, text_dim.b, 1.0f);

        ImGui::PushID(i);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(accent.r, accent.g, accent.b, 0.1f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(accent.r, accent.g, accent.b, 0.2f));
        ImGui::PushStyleColor(ImGuiCol_Text, txt);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(font_size_tabs * 0.75f, (TAB_HEIGHT - ImGui::GetTextLineHeight()) * 0.5f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

        if (ImGui::Button(t.label)) {
            _active_tab = t.tab;
        }

        if (is_active) {
            float indicator_h = font_size_tabs * 0.125f;
            ImVec2 min = ImGui::GetItemRectMin();
            ImVec2 max = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImVec2(min.x, max.y - indicator_h),
                ImVec2(max.x, max.y),
                ImGui::ColorConvertFloat4ToU32(ImVec4(accent.r, accent.g, accent.b, 1.0f))
            );
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);
        ImGui::PopID();
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void BottomPanel::draw_terminal() {
    Color text_dim = _theme ? _theme->get_text_dim() : Color(0.5f, 0.5f, 0.52f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(text_dim.r, text_dim.g, text_dim.b, 1.0f));
    ImGui::TextUnformatted("Terminal ready.");
    ImGui::PopStyleColor();
}

void BottomPanel::draw_output() {
    Color text_dim = _theme ? _theme->get_text_dim() : Color(0.5f, 0.5f, 0.52f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(text_dim.r, text_dim.g, text_dim.b, 1.0f));
    ImGui::TextUnformatted("No output.");
    ImGui::PopStyleColor();
}

void BottomPanel::draw_problems() {
    Color text_dim = _theme ? _theme->get_text_dim() : Color(0.5f, 0.5f, 0.52f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(text_dim.r, text_dim.g, text_dim.b, 1.0f));
    ImGui::TextUnformatted("No problems detected.");
    ImGui::PopStyleColor();
}

}
