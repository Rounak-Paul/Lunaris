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

    float cr = 0, cg = 0, cb = 0;
    if (h < 60) { cr = c; cg = x; cb = 0; }
    else if (h < 120) { cr = x; cg = c; cb = 0; }
    else if (h < 180) { cr = 0; cg = c; cb = x; }
    else if (h < 240) { cr = 0; cg = x; cb = c; }
    else if (h < 300) { cr = x; cg = 0; cb = c; }
    else { cr = c; cg = 0; cb = x; }

    return Color(cr + m, cg + m, cb + m, a);
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

Theme::Theme() : _active_name("Default") {
    _config = make_default();
}

Theme::~Theme() {
}

void Theme::set_config(const ThemeConfig& config) {
    _config = config;
}

void Theme::apply() {
    ImGuiStyle& style = ImGui::GetStyle();
    const auto& p = _config.palette;

    style.WindowPadding = ImVec2(_config.window.padding, _config.window.padding);
    style.FramePadding = ImVec2(_config.button.padding_x, _config.button.padding_y);
    style.ItemSpacing = ImVec2(_config.item_spacing_x, _config.item_spacing_y);
    style.ItemInnerSpacing = ImVec2(_config.item_spacing_x * 0.5f, _config.item_spacing_y * 0.5f);
    style.IndentSpacing = _config.indent_spacing;
    style.ScrollbarSize = _config.scrollbar.width;
    style.GrabMinSize = 12.0f;

    style.WindowBorderSize = _config.window.border_thickness;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = _config.button.border_thickness;
    style.TabBorderSize = 0.0f;

    style.WindowRounding = _config.window.rounding;
    style.ChildRounding = _config.window.rounding;
    style.FrameRounding = _config.button.rounding;
    style.PopupRounding = _config.menu.rounding;
    style.ScrollbarRounding = _config.scrollbar.rounding;
    style.GrabRounding = _config.slider.rounding;
    style.TabRounding = _config.tab.rounding;

    auto to_vec4 = [](const Color& c) { return ImVec4(c.r, c.g, c.b, c.a); };

    style.Colors[ImGuiCol_Text] = to_vec4(p.foreground);
    style.Colors[ImGuiCol_TextDisabled] = to_vec4(p.foreground_disabled);
    style.Colors[ImGuiCol_WindowBg] = to_vec4(p.background);
    style.Colors[ImGuiCol_ChildBg] = to_vec4(p.background);
    style.Colors[ImGuiCol_PopupBg] = to_vec4(p.background_alt);
    style.Colors[ImGuiCol_Border] = to_vec4(p.border);
    style.Colors[ImGuiCol_BorderShadow] = to_vec4(p.shadow);
    style.Colors[ImGuiCol_FrameBg] = to_vec4(_config.input.background);
    style.Colors[ImGuiCol_FrameBgHovered] = to_vec4(_config.input.background_hover);
    style.Colors[ImGuiCol_FrameBgActive] = to_vec4(_config.input.background_focused);
    style.Colors[ImGuiCol_TitleBg] = to_vec4(_config.window.title_background);
    style.Colors[ImGuiCol_TitleBgActive] = to_vec4(_config.window.title_background_active);
    style.Colors[ImGuiCol_TitleBgCollapsed] = to_vec4(_config.window.title_background);
    style.Colors[ImGuiCol_MenuBarBg] = to_vec4(_config.menu.background);
    style.Colors[ImGuiCol_ScrollbarBg] = to_vec4(_config.scrollbar.track);
    style.Colors[ImGuiCol_ScrollbarGrab] = to_vec4(_config.scrollbar.thumb);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = to_vec4(_config.scrollbar.thumb_hover);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = to_vec4(_config.scrollbar.thumb_active);
    style.Colors[ImGuiCol_CheckMark] = to_vec4(p.accent);
    style.Colors[ImGuiCol_SliderGrab] = to_vec4(_config.slider.foreground);
    style.Colors[ImGuiCol_SliderGrabActive] = to_vec4(_config.slider.foreground_active);
    style.Colors[ImGuiCol_Button] = to_vec4(_config.button.background);
    style.Colors[ImGuiCol_ButtonHovered] = to_vec4(_config.button.background_hover);
    style.Colors[ImGuiCol_ButtonActive] = to_vec4(_config.button.background_active);
    style.Colors[ImGuiCol_Header] = to_vec4(p.background_highlight);
    style.Colors[ImGuiCol_HeaderHovered] = to_vec4(p.background_highlight.lighten(0.1f));
    style.Colors[ImGuiCol_HeaderActive] = to_vec4(p.accent);
    style.Colors[ImGuiCol_Separator] = to_vec4(p.border);
    style.Colors[ImGuiCol_SeparatorHovered] = to_vec4(p.accent);
    style.Colors[ImGuiCol_SeparatorActive] = to_vec4(p.accent_active);
    style.Colors[ImGuiCol_ResizeGrip] = to_vec4(p.border);
    style.Colors[ImGuiCol_ResizeGripHovered] = to_vec4(p.accent);
    style.Colors[ImGuiCol_ResizeGripActive] = to_vec4(p.accent_active);
    style.Colors[ImGuiCol_Tab] = to_vec4(_config.tab.background);
    style.Colors[ImGuiCol_TabHovered] = to_vec4(_config.tab.background_hover);
    style.Colors[ImGuiCol_TabSelected] = to_vec4(_config.tab.background_active);
    style.Colors[ImGuiCol_TabSelectedOverline] = to_vec4(_config.tab.underline);
    style.Colors[ImGuiCol_TabDimmed] = to_vec4(_config.tab.background);
    style.Colors[ImGuiCol_TabDimmedSelected] = to_vec4(_config.tab.background_active);
    style.Colors[ImGuiCol_TabDimmedSelectedOverline] = to_vec4(_config.tab.underline.with_alpha(0.5f));
    style.Colors[ImGuiCol_DockingPreview] = to_vec4(p.accent.with_alpha(0.5f));
    style.Colors[ImGuiCol_DockingEmptyBg] = to_vec4(p.background_alt);
    style.Colors[ImGuiCol_PlotLines] = to_vec4(p.accent);
    style.Colors[ImGuiCol_PlotLinesHovered] = to_vec4(p.accent_hover);
    style.Colors[ImGuiCol_PlotHistogram] = to_vec4(p.accent);
    style.Colors[ImGuiCol_PlotHistogramHovered] = to_vec4(p.accent_hover);
    style.Colors[ImGuiCol_TableHeaderBg] = to_vec4(p.background_alt);
    style.Colors[ImGuiCol_TableBorderStrong] = to_vec4(p.border);
    style.Colors[ImGuiCol_TableBorderLight] = to_vec4(p.border.with_alpha(0.5f));
    style.Colors[ImGuiCol_TableRowBg] = to_vec4(p.background);
    style.Colors[ImGuiCol_TableRowBgAlt] = to_vec4(p.background_alt);
    style.Colors[ImGuiCol_TextSelectedBg] = to_vec4(p.selection);
    style.Colors[ImGuiCol_DragDropTarget] = to_vec4(p.accent);
    style.Colors[ImGuiCol_NavCursor] = to_vec4(p.accent);
    style.Colors[ImGuiCol_NavWindowingHighlight] = to_vec4(p.accent.with_alpha(0.7f));
    style.Colors[ImGuiCol_NavWindowingDimBg] = to_vec4(p.shadow);
    style.Colors[ImGuiCol_ModalWindowDimBg] = to_vec4(Color(0.0f, 0.0f, 0.0f, 0.6f));
}

ThemeConfig Theme::make_default() {
    ThemeConfig cfg;
    ColorPalette& p = cfg.palette;
    
    p.background = Color::rgb(30, 30, 30);
    p.background_alt = Color::rgb(37, 37, 38);
    p.background_highlight = Color::rgb(45, 45, 48);
    
    p.foreground = Color::rgb(212, 212, 212);
    p.foreground_dim = Color::rgb(128, 128, 128);
    p.foreground_disabled = Color::rgb(90, 90, 90);
    
    p.border = Color::rgb(60, 60, 60);
    p.border_light = Color::rgb(80, 80, 80);
    p.border_focus = Color::rgb(0, 122, 204);
    
    p.accent = Color::rgb(0, 122, 204);
    p.accent_hover = Color::rgb(28, 151, 234);
    p.accent_active = Color::rgb(0, 102, 184);
    
    p.success = Color::rgb(78, 154, 6);
    p.warning = Color::rgb(196, 160, 0);
    p.error = Color::rgb(204, 0, 0);
    p.info = Color::rgb(52, 101, 164);
    
    p.selection = Color(0.0f, 0.48f, 0.8f, 0.3f);
    p.shadow = Color(0.0f, 0.0f, 0.0f, 0.5f);

    cfg.button.background = p.background_alt;
    cfg.button.background_hover = p.background_highlight;
    cfg.button.background_active = p.accent;
    cfg.button.background_disabled = p.background;
    cfg.button.foreground = p.foreground;
    cfg.button.foreground_hover = p.foreground;
    cfg.button.foreground_active = p.foreground;
    cfg.button.foreground_disabled = p.foreground_disabled;
    cfg.button.border = p.border;
    cfg.button.border_hover = p.border_light;
    cfg.button.border_active = p.accent;
    cfg.button.border_disabled = p.border;
    cfg.button.border_thickness = 1.0f;
    cfg.button.rounding = 2.0f;
    cfg.button.padding_x = 8.0f;
    cfg.button.padding_y = 4.0f;
    cfg.button.border_style = BorderStyle::Solid;
    
    cfg.checkbox = cfg.button;
    cfg.checkbox.padding_x = 2.0f;
    cfg.checkbox.padding_y = 2.0f;
    
    cfg.radio = cfg.checkbox;
    
    cfg.slider = cfg.button;
    cfg.slider.foreground = p.accent;
    cfg.slider.foreground_hover = p.accent_hover;
    cfg.slider.foreground_active = p.accent_active;
    cfg.slider.padding_x = 0.0f;
    cfg.slider.padding_y = 0.0f;
    
    cfg.input.background = p.background;
    cfg.input.background_hover = p.background;
    cfg.input.background_focused = p.background;
    cfg.input.foreground = p.foreground;
    cfg.input.placeholder = p.foreground_dim;
    cfg.input.selection = p.selection;
    cfg.input.cursor = p.accent;
    cfg.input.border = p.border;
    cfg.input.border_hover = p.border_light;
    cfg.input.border_focused = p.border_focus;
    cfg.input.border_thickness = 1.0f;
    cfg.input.rounding = 2.0f;
    cfg.input.padding_x = 6.0f;
    cfg.input.padding_y = 4.0f;
    cfg.input.border_style = BorderStyle::Solid;
    
    cfg.window.background = p.background;
    cfg.window.title_background = p.background_alt;
    cfg.window.title_background_active = p.background_highlight;
    cfg.window.title_foreground = p.foreground;
    cfg.window.border = p.border;
    cfg.window.border_thickness = 1.0f;
    cfg.window.rounding = 4.0f;
    cfg.window.padding = 8.0f;
    cfg.window.title_height = 22.0f;
    cfg.window.border_style = BorderStyle::Solid;
    
    cfg.scrollbar.track = p.background;
    cfg.scrollbar.thumb = p.border_light;
    cfg.scrollbar.thumb_hover = p.foreground_dim;
    cfg.scrollbar.thumb_active = p.accent;
    cfg.scrollbar.width = 10.0f;
    cfg.scrollbar.rounding = 2.0f;
    cfg.scrollbar.min_thumb_size = 20.0f;
    
    cfg.menu.background = p.background_alt;
    cfg.menu.item_hover = p.background_highlight;
    cfg.menu.item_active = p.accent;
    cfg.menu.foreground = p.foreground;
    cfg.menu.foreground_hover = p.foreground;
    cfg.menu.foreground_disabled = p.foreground_disabled;
    cfg.menu.separator = p.border;
    cfg.menu.border = p.border;
    cfg.menu.border_thickness = 1.0f;
    cfg.menu.rounding = 2.0f;
    cfg.menu.item_padding_x = 8.0f;
    cfg.menu.item_padding_y = 4.0f;
    
    cfg.tab.background = p.background;
    cfg.tab.background_hover = p.background_alt;
    cfg.tab.background_active = p.background_alt;
    cfg.tab.foreground = p.foreground_dim;
    cfg.tab.foreground_active = p.foreground;
    cfg.tab.border = p.border;
    cfg.tab.underline = p.accent;
    cfg.tab.rounding = 2.0f;
    cfg.tab.padding_x = 10.0f;
    cfg.tab.padding_y = 4.0f;
    
    cfg.font_size = 14.0f;
    cfg.item_spacing_x = 8.0f;
    cfg.item_spacing_y = 4.0f;
    cfg.indent_spacing = 20.0f;
    cfg.pixel_perfect = false;

    return cfg;
}

Theme* Theme::get_default() {
    if (!_default_instance) {
        _default_instance = new Theme();
    }
    return _default_instance;
}

}
