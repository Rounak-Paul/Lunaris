#pragma once

#include <cstdint>

namespace lunaris {

struct Color {
    float r, g, b, a;

    Color() : r(0), g(0), b(0), a(1) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

    static Color rgb(uint8_t ri, uint8_t gi, uint8_t bi, uint8_t ai = 255) {
        return Color(ri / 255.0f, gi / 255.0f, bi / 255.0f, ai / 255.0f);
    }

    static Color from_hex(uint32_t hex);
    static Color from_hsv(float h, float s, float v, float a = 1.0f);
    uint32_t to_hex() const;
    void to_hsv(float& h, float& s, float& v) const;

    Color lighten(float amount) const;
    Color darken(float amount) const;
    Color with_alpha(float alpha) const;
};

enum class BorderStyle : uint8_t {
    None,
    Solid,
    Raised,
    Sunken
};

struct ColorPalette {
    Color background;
    Color background_alt;
    Color background_highlight;
    Color foreground;
    Color foreground_dim;
    Color foreground_disabled;
    Color border;
    Color border_light;
    Color border_focus;
    Color accent;
    Color accent_hover;
    Color accent_active;
    Color success;
    Color warning;
    Color error;
    Color info;
    Color selection;
    Color shadow;
};

struct ControlStyle {
    Color background;
    Color background_hover;
    Color background_active;
    Color background_disabled;
    Color foreground;
    Color foreground_hover;
    Color foreground_active;
    Color foreground_disabled;
    Color border;
    Color border_hover;
    Color border_active;
    Color border_disabled;
    float border_thickness;
    float rounding;
    float padding_x;
    float padding_y;
    BorderStyle border_style;
};

struct InputStyle {
    Color background;
    Color background_hover;
    Color background_focused;
    Color foreground;
    Color placeholder;
    Color selection;
    Color cursor;
    Color border;
    Color border_hover;
    Color border_focused;
    float border_thickness;
    float rounding;
    float padding_x;
    float padding_y;
    BorderStyle border_style;
};

struct WindowStyle {
    Color background;
    Color title_background;
    Color title_background_active;
    Color title_foreground;
    Color border;
    float border_thickness;
    float rounding;
    float padding;
    float title_height;
    BorderStyle border_style;
};

struct ScrollbarStyle {
    Color track;
    Color thumb;
    Color thumb_hover;
    Color thumb_active;
    float width;
    float rounding;
    float min_thumb_size;
};

struct MenuStyle {
    Color background;
    Color item_hover;
    Color item_active;
    Color foreground;
    Color foreground_hover;
    Color foreground_disabled;
    Color separator;
    Color border;
    float border_thickness;
    float rounding;
    float item_padding_x;
    float item_padding_y;
};

struct TabStyle {
    Color background;
    Color background_hover;
    Color background_active;
    Color foreground;
    Color foreground_active;
    Color border;
    Color underline;
    float rounding;
    float padding_x;
    float padding_y;
};

struct ThemeConfig {
    ColorPalette palette;
    ControlStyle button;
    ControlStyle checkbox;
    ControlStyle radio;
    ControlStyle slider;
    InputStyle input;
    WindowStyle window;
    ScrollbarStyle scrollbar;
    MenuStyle menu;
    TabStyle tab;
    float font_size;
    float item_spacing_x;
    float item_spacing_y;
    float indent_spacing;
    bool pixel_perfect;
};

class Theme {
public:
    Theme();
    ~Theme();

    void set_config(const ThemeConfig& config);
    void apply();

    const ThemeConfig& config() const { return _config; }
    const char* active_theme_name() const { return _active_name; }
    void set_active_name(const char* name) { _active_name = name; }

    Color get_background() const { return _config.palette.background; }
    Color get_background_alt() const { return _config.palette.background_alt; }
    Color get_surface() const { return _config.palette.background_highlight; }
    Color get_text() const { return _config.palette.foreground; }
    Color get_text_dim() const { return _config.palette.foreground_dim; }
    Color get_border() const { return _config.palette.border; }
    Color get_accent() const { return _config.palette.accent; }
    Color get_accent_hover() const { return _config.palette.accent_hover; }
    Color get_accent_active() const { return _config.palette.accent_active; }
    Color get_success() const { return _config.palette.success; }
    Color get_warning() const { return _config.palette.warning; }
    Color get_error() const { return _config.palette.error; }

    static ThemeConfig make_default();
    static Theme* get_default();

private:
    ThemeConfig _config;
    const char* _active_name;
    static Theme* _default_instance;
};

}
