#include "lunaris/core/theme.h"
#include <imgui.h>
#include <algorithm>
#include <cmath>

namespace lunaris {

Theme* Theme::_default_instance = nullptr;

Color Color::from_hex(uint32_t hex) {
    return Color(
        ((hex >> 16) & 0xFF) / 255.0f,
        ((hex >> 8) & 0xFF) / 255.0f,
        (hex & 0xFF) / 255.0f,
        1.0f
    );
}

Color Color::from_hsv(float h, float s, float v, float a) {
    float c = v * s;
    float x = c * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;

    float r = 0, g = 0, b = 0;
    if (h < 60) { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    return Color(r + m, g + m, b + m, a);
}

uint32_t Color::to_hex() const {
    uint32_t ri = static_cast<uint32_t>(std::clamp(r, 0.0f, 1.0f) * 255.0f);
    uint32_t gi = static_cast<uint32_t>(std::clamp(g, 0.0f, 1.0f) * 255.0f);
    uint32_t bi = static_cast<uint32_t>(std::clamp(b, 0.0f, 1.0f) * 255.0f);
    return (ri << 16) | (gi << 8) | bi;
}

void Color::to_hsv(float& h, float& s, float& v) const {
    float max_c = std::max({r, g, b});
    float min_c = std::min({r, g, b});
    float delta = max_c - min_c;

    v = max_c;

    if (max_c > 0.0f) {
        s = delta / max_c;
    } else {
        s = 0.0f;
        h = 0.0f;
        return;
    }

    if (delta < 0.00001f) {
        h = 0.0f;
        return;
    }

    if (r >= max_c) {
        h = 60.0f * std::fmod((g - b) / delta, 6.0f);
    } else if (g >= max_c) {
        h = 60.0f * (((b - r) / delta) + 2.0f);
    } else {
        h = 60.0f * (((r - g) / delta) + 4.0f);
    }

    if (h < 0.0f) {
        h += 360.0f;
    }
}

Color Color::lighten(float amount) const {
    float h, s, v;
    to_hsv(h, s, v);
    v = std::clamp(v + amount, 0.0f, 1.0f);
    s = std::clamp(s - amount * 0.3f, 0.0f, 1.0f);
    return Color::from_hsv(h, s, v, a);
}

Color Color::darken(float amount) const {
    float h, s, v;
    to_hsv(h, s, v);
    v = std::clamp(v - amount, 0.0f, 1.0f);
    return Color::from_hsv(h, s, v, a);
}

Color Color::with_alpha(float alpha) const {
    return Color(r, g, b, alpha);
}

Theme::Theme()
    : _primary(Color::from_hex(0x4A9EFF))
    , _style(ThemeStyle::Dark) {
    generate_palette();
}

Theme::~Theme() {
}

void Theme::set_primary_color(const Color& color) {
    _primary = color;
    generate_palette();
}

void Theme::set_primary_color_hex(uint32_t hex) {
    _primary = Color::from_hex(hex);
    generate_palette();
}

void Theme::set_style(ThemeStyle style) {
    _style = style;
    generate_palette();
}

void Theme::generate_palette() {
    if (_style == ThemeStyle::Dark) {
        _background = Color(0.08f, 0.08f, 0.09f, 1.0f);
        _background_alt = Color(0.11f, 0.11f, 0.12f, 1.0f);
        _surface = Color(0.14f, 0.14f, 0.16f, 1.0f);
        _text = Color(0.92f, 0.92f, 0.94f, 1.0f);
        _text_dim = Color(0.55f, 0.55f, 0.58f, 1.0f);
        _border = Color(0.22f, 0.22f, 0.25f, 1.0f);
    } else {
        _background = Color(0.96f, 0.96f, 0.97f, 1.0f);
        _background_alt = Color(0.92f, 0.92f, 0.94f, 1.0f);
        _surface = Color(1.0f, 1.0f, 1.0f, 1.0f);
        _text = Color(0.1f, 0.1f, 0.12f, 1.0f);
        _text_dim = Color(0.45f, 0.45f, 0.5f, 1.0f);
        _border = Color(0.8f, 0.8f, 0.82f, 1.0f);
    }

    _accent = _primary;
    _accent_hover = _primary.lighten(0.15f);
    _accent_active = _primary.darken(0.1f);

    _success = Color::from_hex(0x4CAF50);
    _warning = Color::from_hex(0xFFA726);
    _error = Color::from_hex(0xEF5350);
}

void Theme::apply() {
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 3.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;

    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);

    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text] = ImVec4(_text.r, _text.g, _text.b, _text.a);
    colors[ImGuiCol_TextDisabled] = ImVec4(_text_dim.r, _text_dim.g, _text_dim.b, _text_dim.a);

    colors[ImGuiCol_WindowBg] = ImVec4(_background.r, _background.g, _background.b, 1.0f);
    colors[ImGuiCol_ChildBg] = ImVec4(_background.r, _background.g, _background.b, 0.0f);
    colors[ImGuiCol_PopupBg] = ImVec4(_surface.r, _surface.g, _surface.b, 0.98f);

    colors[ImGuiCol_Border] = ImVec4(_border.r, _border.g, _border.b, 0.6f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    colors[ImGuiCol_FrameBg] = ImVec4(_surface.r, _surface.g, _surface.b, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(_accent.r, _accent.g, _accent.b, 0.2f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(_accent.r, _accent.g, _accent.b, 0.35f);

    colors[ImGuiCol_TitleBg] = ImVec4(_background_alt.r, _background_alt.g, _background_alt.b, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(_surface.r, _surface.g, _surface.b, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(_background.r, _background.g, _background.b, 0.8f);

    colors[ImGuiCol_MenuBarBg] = ImVec4(_background_alt.r, _background_alt.g, _background_alt.b, 1.0f);

    colors[ImGuiCol_ScrollbarBg] = ImVec4(_background.r, _background.g, _background.b, 0.5f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(_border.r, _border.g, _border.b, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(_accent.r, _accent.g, _accent.b, 0.7f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(_accent.r, _accent.g, _accent.b, 1.0f);

    colors[ImGuiCol_CheckMark] = ImVec4(_accent.r, _accent.g, _accent.b, 1.0f);
    colors[ImGuiCol_SliderGrab] = ImVec4(_accent.r, _accent.g, _accent.b, 0.8f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(_accent_active.r, _accent_active.g, _accent_active.b, 1.0f);

    colors[ImGuiCol_Button] = ImVec4(_surface.r, _surface.g, _surface.b, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(_accent.r, _accent.g, _accent.b, 0.8f);
    colors[ImGuiCol_ButtonActive] = ImVec4(_accent_active.r, _accent_active.g, _accent_active.b, 1.0f);

    colors[ImGuiCol_Header] = ImVec4(_accent.r, _accent.g, _accent.b, 0.25f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(_accent.r, _accent.g, _accent.b, 0.5f);
    colors[ImGuiCol_HeaderActive] = ImVec4(_accent.r, _accent.g, _accent.b, 0.7f);

    colors[ImGuiCol_Separator] = ImVec4(_border.r, _border.g, _border.b, 0.6f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(_accent.r, _accent.g, _accent.b, 0.7f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(_accent.r, _accent.g, _accent.b, 1.0f);

    colors[ImGuiCol_ResizeGrip] = ImVec4(_accent.r, _accent.g, _accent.b, 0.2f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(_accent.r, _accent.g, _accent.b, 0.6f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(_accent.r, _accent.g, _accent.b, 0.9f);

    colors[ImGuiCol_Tab] = ImVec4(_background_alt.r, _background_alt.g, _background_alt.b, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(_accent.r, _accent.g, _accent.b, 0.7f);
    colors[ImGuiCol_TabActive] = ImVec4(_accent.r, _accent.g, _accent.b, 0.9f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(_background_alt.r, _background_alt.g, _background_alt.b, 0.9f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(_surface.r, _surface.g, _surface.b, 1.0f);

    colors[ImGuiCol_DockingPreview] = ImVec4(_accent.r, _accent.g, _accent.b, 0.5f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(_background.r, _background.g, _background.b, 1.0f);

    colors[ImGuiCol_PlotLines] = ImVec4(_accent.r, _accent.g, _accent.b, 1.0f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(_accent_hover.r, _accent_hover.g, _accent_hover.b, 1.0f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(_accent.r, _accent.g, _accent.b, 1.0f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(_accent_hover.r, _accent_hover.g, _accent_hover.b, 1.0f);

    colors[ImGuiCol_TableHeaderBg] = ImVec4(_background_alt.r, _background_alt.g, _background_alt.b, 1.0f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(_border.r, _border.g, _border.b, 1.0f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(_border.r, _border.g, _border.b, 0.5f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(_surface.r, _surface.g, _surface.b, 0.3f);

    colors[ImGuiCol_TextSelectedBg] = ImVec4(_accent.r, _accent.g, _accent.b, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(_accent.r, _accent.g, _accent.b, 0.9f);

    colors[ImGuiCol_NavHighlight] = ImVec4(_accent.r, _accent.g, _accent.b, 1.0f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
}

Theme* Theme::get_default() {
    if (!_default_instance) {
        _default_instance = new Theme();
    }
    return _default_instance;
}

}
