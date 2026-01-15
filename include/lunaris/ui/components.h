#pragma once

#include <imgui.h>

namespace lunaris {

class Theme;

namespace ui {

struct TooltipConfig {
    float delay;
    float padding_x;
    float padding_y;
    float rounding;
    bool show_shortcut;
};

struct IconButtonConfig {
    float size;
    float icon_size;
    float rounding;
    bool show_active_indicator;
    float indicator_width;
};

void init(Theme* theme);
void shutdown();

void set_theme(Theme* theme);
Theme* get_theme();

void tooltip(const char* text, const char* shortcut = nullptr);
void tooltip_delayed(const char* text, const char* shortcut = nullptr, float delay = 0.5f);

bool icon_button(const char* icon, const char* id, bool is_active = false, float size = 36.0f);
bool icon_button_with_tooltip(const char* icon, const char* id, const char* tooltip_text, 
                              const char* shortcut = nullptr, bool is_active = false, float size = 36.0f);

void draw_active_indicator(float height, float width = 3.0f);

void draw_raised_border(ImVec2 min, ImVec2 max, float thickness = 2.0f);
void draw_sunken_border(ImVec2 min, ImVec2 max, float thickness = 2.0f);
bool raised_button(const char* label, ImVec2 size = ImVec2(0, 0));
bool raised_icon_button(const char* icon, ImVec2 size = ImVec2(0, 0));

void begin_disabled(bool disabled = true);
void end_disabled();

void push_accent_button_style();
void pop_accent_button_style();

void push_transparent_button_style();
void pop_transparent_button_style();

float get_content_width();
float get_content_height();

}

}
