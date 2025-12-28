#pragma once

#include <cstdint>

namespace lunaris {

struct Color {
    float r, g, b, a;

    Color() : r(0), g(0), b(0), a(1) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

    static Color from_hex(uint32_t hex);
    static Color from_hsv(float h, float s, float v, float a = 1.0f);
    uint32_t to_hex() const;
    void to_hsv(float& h, float& s, float& v) const;

    Color lighten(float amount) const;
    Color darken(float amount) const;
    Color with_alpha(float alpha) const;
};

enum class ThemeStyle : uint8_t {
    Dark,
    Light
};

class Theme {
public:
    Theme();
    ~Theme();

    void set_primary_color(const Color& color);
    void set_primary_color_hex(uint32_t hex);
    void set_style(ThemeStyle style);

    void set_background(const Color& color) { _background = color; }
    void set_background_alt(const Color& color) { _background_alt = color; }
    void set_surface(const Color& color) { _surface = color; }
    void set_text(const Color& color) { _text = color; }
    void set_text_dim(const Color& color) { _text_dim = color; }
    void set_border(const Color& color) { _border = color; }
    void set_accent(const Color& color) { _accent = color; }
    void set_accent_hover(const Color& color) { _accent_hover = color; }
    void set_accent_active(const Color& color) { _accent_active = color; }
    void set_success(const Color& color) { _success = color; }
    void set_warning(const Color& color) { _warning = color; }
    void set_error(const Color& color) { _error = color; }

    void apply();

    Color get_primary() const { return _primary; }
    Color get_background() const { return _background; }
    Color get_background_alt() const { return _background_alt; }
    Color get_surface() const { return _surface; }
    Color get_text() const { return _text; }
    Color get_text_dim() const { return _text_dim; }
    Color get_border() const { return _border; }
    Color get_accent() const { return _accent; }
    Color get_accent_hover() const { return _accent_hover; }
    Color get_accent_active() const { return _accent_active; }
    Color get_success() const { return _success; }
    Color get_warning() const { return _warning; }
    Color get_error() const { return _error; }

    ThemeStyle get_style() const { return _style; }

    static Theme* get_default();

private:
    void generate_palette();

    Color _primary;
    ThemeStyle _style;

    Color _background;
    Color _background_alt;
    Color _surface;
    Color _text;
    Color _text_dim;
    Color _border;
    Color _accent;
    Color _accent_hover;
    Color _accent_active;
    Color _success;
    Color _warning;
    Color _error;

    static Theme* _default_instance;
};

}
