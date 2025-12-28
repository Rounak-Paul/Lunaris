#include "lunaris/ui/components.h"
#include "lunaris/core/theme.h"
#include <imgui_internal.h>

namespace lunaris {
namespace ui {

static Theme* s_theme = nullptr;
static float s_hover_start_time = 0.0f;
static ImGuiID s_last_hovered_id = 0;

void init(Theme* theme) {
    s_theme = theme;
    s_hover_start_time = 0.0f;
    s_last_hovered_id = 0;
}

void shutdown() {
    s_theme = nullptr;
}

void set_theme(Theme* theme) {
    s_theme = theme;
}

Theme* get_theme() {
    return s_theme;
}

void tooltip(const char* text, const char* shortcut) {
    Color bg = s_theme ? s_theme->get_surface().lighten(0.05f) : Color(0.18f, 0.18f, 0.2f);
    Color border = s_theme ? s_theme->get_border() : Color(0.3f, 0.3f, 0.32f);
    Color text_color = s_theme ? s_theme->get_text() : Color(0.9f, 0.9f, 0.92f);
    Color dim_color = s_theme ? s_theme->get_text_dim() : Color(0.6f, 0.6f, 0.62f);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(bg.r, bg.g, bg.b, 0.98f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(border.r, border.g, border.b, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(text_color.r, text_color.g, text_color.b, 1.0f));

    ImGui::BeginTooltip();

    if (shortcut && shortcut[0] != '\0') {
        ImGui::TextUnformatted(text);
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(dim_color.r, dim_color.g, dim_color.b, 1.0f));
        ImGui::Text("(%s)", shortcut);
        ImGui::PopStyleColor();
    } else {
        ImGui::TextUnformatted(text);
    }

    ImGui::EndTooltip();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(3);
}

void tooltip_delayed(const char* text, const char* shortcut, float delay) {
    ImGuiID current_id = ImGui::GetItemID();
    float current_time = static_cast<float>(ImGui::GetTime());

    if (current_id != s_last_hovered_id) {
        s_last_hovered_id = current_id;
        s_hover_start_time = current_time;
    }

    if (current_time - s_hover_start_time >= delay) {
        tooltip(text, shortcut);
    }
}

bool icon_button(const char* icon, const char* id, bool is_active, float size) {
    Color text = s_theme ? s_theme->get_text() : Color(0.9f, 0.9f, 0.92f);
    Color text_dim = s_theme ? s_theme->get_text_dim() : Color(0.5f, 0.5f, 0.52f);
    Color accent = s_theme ? s_theme->get_accent() : Color(0.3f, 0.5f, 0.8f);
    Color hover = s_theme ? s_theme->get_accent_hover() : Color(0.35f, 0.55f, 0.85f);

    ImVec4 text_color = is_active 
        ? ImVec4(text.r, text.g, text.b, 1.0f)
        : ImVec4(text_dim.r, text_dim.g, text_dim.b, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(hover.r, hover.g, hover.b, 0.15f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(accent.r, accent.g, accent.b, 0.25f));
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

    ImGui::PushID(id);
    bool clicked = ImGui::Button(icon, ImVec2(size, size));
    ImGui::PopID();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);

    return clicked;
}

bool icon_button_with_tooltip(const char* icon, const char* id, const char* tooltip_text, 
                              const char* shortcut, bool is_active, float size) {
    bool clicked = icon_button(icon, id, is_active, size);

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
        tooltip(tooltip_text, shortcut);
    }

    return clicked;
}

void draw_active_indicator(float height, float width) {
    Color accent = s_theme ? s_theme->get_accent() : Color(0.3f, 0.5f, 0.8f);
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float indicator_x = cursor.x - ImGui::GetCursorPosX();
    draw_list->AddRectFilled(
        ImVec2(indicator_x, cursor.y),
        ImVec2(indicator_x + width, cursor.y + height),
        ImGui::ColorConvertFloat4ToU32(ImVec4(accent.r, accent.g, accent.b, 1.0f)),
        width * 0.5f
    );
}

void begin_disabled(bool disabled) {
    if (disabled) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    }
}

void end_disabled() {
    ImGui::PopStyleVar();
    ImGui::PopItemFlag();
}

void push_accent_button_style() {
    Color accent = s_theme ? s_theme->get_accent() : Color(0.3f, 0.5f, 0.8f);
    Color hover = s_theme ? s_theme->get_accent_hover() : Color(0.35f, 0.55f, 0.85f);
    Color active = s_theme ? s_theme->get_accent_active() : Color(0.25f, 0.45f, 0.75f);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(accent.r, accent.g, accent.b, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(hover.r, hover.g, hover.b, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(active.r, active.g, active.b, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
}

void pop_accent_button_style() {
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
}

void push_transparent_button_style() {
    Color accent = s_theme ? s_theme->get_accent() : Color(0.3f, 0.5f, 0.8f);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(accent.r, accent.g, accent.b, 0.15f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(accent.r, accent.g, accent.b, 0.25f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
}

void pop_transparent_button_style() {
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
}

float get_content_width() {
    return ImGui::GetContentRegionAvail().x;
}

float get_content_height() {
    return ImGui::GetContentRegionAvail().y;
}

}
}
