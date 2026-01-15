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

    float font_size = ImGui::GetFontSize();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(font_size * 0.625f, font_size * 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
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
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

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

void draw_raised_border(ImVec2 min, ImVec2 max, float thickness) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    Color light = s_theme ? s_theme->config().palette.border_light : Color::rgb(100, 100, 110);
    Color dark = s_theme ? s_theme->config().palette.border : Color::rgb(20, 20, 25);
    
    ImU32 light_col = IM_COL32((int)(light.r*255), (int)(light.g*255), (int)(light.b*255), 255);
    ImU32 dark_col = IM_COL32((int)(dark.r*255), (int)(dark.g*255), (int)(dark.b*255), 255);
    
    float half = thickness * 0.5f;
    draw_list->AddLine(ImVec2(min.x, max.y - half), ImVec2(max.x, max.y - half), dark_col, thickness);
    draw_list->AddLine(ImVec2(max.x - half, min.y), ImVec2(max.x - half, max.y), dark_col, thickness);
    
    draw_list->AddLine(ImVec2(min.x, min.y + half), ImVec2(max.x - thickness, min.y + half), light_col, thickness);
    draw_list->AddLine(ImVec2(min.x + half, min.y), ImVec2(min.x + half, max.y - thickness), light_col, thickness);
}

void draw_sunken_border(ImVec2 min, ImVec2 max, float thickness) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    Color light = s_theme ? s_theme->config().palette.border_light : Color::rgb(100, 100, 110);
    Color dark = s_theme ? s_theme->config().palette.border : Color::rgb(20, 20, 25);
    
    ImU32 light_col = IM_COL32((int)(light.r*255), (int)(light.g*255), (int)(light.b*255), 255);
    ImU32 dark_col = IM_COL32((int)(dark.r*255), (int)(dark.g*255), (int)(dark.b*255), 255);
    
    float half = thickness * 0.5f;
    draw_list->AddLine(ImVec2(min.x, min.y + half), ImVec2(max.x, min.y + half), dark_col, thickness);
    draw_list->AddLine(ImVec2(min.x + half, min.y), ImVec2(min.x + half, max.y), dark_col, thickness);
    
    draw_list->AddLine(ImVec2(min.x, max.y - half), ImVec2(max.x, max.y - half), light_col, thickness);
    draw_list->AddLine(ImVec2(max.x - half, min.y), ImVec2(max.x - half, max.y), light_col, thickness);
}

bool raised_button(const char* label, ImVec2 size) {
    const ThemeConfig& cfg = s_theme ? s_theme->config() : ThemeConfig();
    
    float font_size = ImGui::GetFontSize();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    
    if (size.x == 0) {
        size.x = ImGui::CalcTextSize(label).x + cfg.button.padding_x * 2 + font_size * 0.25f;
    }
    if (size.y == 0) {
        size.y = ImGui::CalcTextSize(label).y + cfg.button.padding_y * 2 + font_size * 0.25f;
    }
    
    Color face_c = cfg.button.background;
    Color face_pressed_c = cfg.button.background_active;
    Color text_c = cfg.button.foreground;
    
    ImU32 face = IM_COL32((int)(face_c.r*255), (int)(face_c.g*255), (int)(face_c.b*255), 255);
    ImU32 face_pressed = IM_COL32((int)(face_pressed_c.r*255), (int)(face_pressed_c.g*255), (int)(face_pressed_c.b*255), 255);
    ImU32 text_col = IM_COL32((int)(text_c.r*255), (int)(text_c.g*255), (int)(text_c.b*255), 255);
    
    ImGui::InvisibleButton(label, size);
    bool clicked = ImGui::IsItemClicked();
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();
    (void)hovered;
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 min = pos;
    ImVec2 max = ImVec2(pos.x + size.x, pos.y + size.y);
    
    draw_list->AddRectFilled(min, max, active ? face_pressed : face);
    
    float border_t = font_size * 0.125f;
    if (active) {
        draw_sunken_border(min, max, border_t);
    } else {
        draw_raised_border(min, max, border_t);
    }
    
    ImVec2 text_size = ImGui::CalcTextSize(label);
    float text_x = pos.x + (size.x - text_size.x) * 0.5f;
    float text_y = pos.y + (size.y - text_size.y) * 0.5f;
    float press_offset = font_size * 0.0625f;
    if (active) {
        text_x += press_offset;
        text_y += press_offset;
    }
    draw_list->AddText(ImVec2(text_x, text_y), text_col, label);
    
    return clicked;
}

bool raised_icon_button(const char* icon, ImVec2 size) {
    const ThemeConfig& cfg = s_theme ? s_theme->config() : ThemeConfig();
    
    float font_size = ImGui::GetFontSize();
    if (size.x == 0 || size.y == 0) {
        size = ImVec2(font_size * 1.25f, font_size * 1.25f);
    }
    
    const char* display_end = icon;
    while (*display_end && !(display_end[0] == '#' && display_end[1] == '#')) {
        display_end++;
    }
    
    ImVec2 pos = ImGui::GetCursorScreenPos();
    
    Color face_c = cfg.button.background;
    Color face_pressed_c = cfg.button.background_active;
    Color text_c = cfg.button.foreground;
    
    ImU32 face = IM_COL32((int)(face_c.r*255), (int)(face_c.g*255), (int)(face_c.b*255), 255);
    ImU32 face_pressed = IM_COL32((int)(face_pressed_c.r*255), (int)(face_pressed_c.g*255), (int)(face_pressed_c.b*255), 255);
    ImU32 icon_col = IM_COL32((int)(text_c.r*255), (int)(text_c.g*255), (int)(text_c.b*255), 255);
    
    ImGui::InvisibleButton(icon, size);
    bool clicked = ImGui::IsItemClicked();
    bool active = ImGui::IsItemActive();
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 min = pos;
    ImVec2 max = ImVec2(pos.x + size.x, pos.y + size.y);
    
    draw_list->AddRectFilled(min, max, active ? face_pressed : face);
    
    float border_t = font_size * 0.125f;
    if (active) {
        draw_sunken_border(min, max, border_t);
    } else {
        draw_raised_border(min, max, border_t);
    }
    
    ImVec2 icon_size = ImGui::CalcTextSize(icon, display_end);
    float icon_x = pos.x + (size.x - icon_size.x) * 0.5f;
    float icon_y = pos.y + (size.y - icon_size.y) * 0.5f;
    float press_offset = font_size * 0.0625f;
    if (active) {
        icon_x += press_offset;
        icon_y += press_offset;
    }
    draw_list->AddText(ImVec2(icon_x, icon_y), icon_col, icon, display_end);
    
    return clicked;
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
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
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
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
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
