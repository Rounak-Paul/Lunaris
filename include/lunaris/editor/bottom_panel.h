#pragma once

#include <cstdint>

namespace lunaris {

class Theme;

enum class BottomPanelTab : uint8_t {
    Terminal,
    Output,
    Problems
};

class BottomPanel {
public:
    static constexpr float HEIGHT = 200.0f;
    static constexpr float MIN_HEIGHT = 100.0f;
    static constexpr float MAX_HEIGHT = 500.0f;
    static constexpr float TAB_HEIGHT = 28.0f;

    BottomPanel();
    ~BottomPanel();

    void set_theme(Theme* theme) { _theme = theme; }

    void on_init();
    void on_shutdown();
    void on_ui();

    void toggle() { _visible = !_visible; }
    void show() { _visible = true; }
    void hide() { _visible = false; }
    bool is_visible() const { return _visible; }

    void set_active_tab(BottomPanelTab tab) { _active_tab = tab; }
    BottomPanelTab get_active_tab() const { return _active_tab; }

    float get_height() const { return _visible ? _height : 0.0f; }

private:
    void draw_tabs();
    void draw_terminal();
    void draw_output();
    void draw_problems();

    Theme* _theme;
    bool _visible;
    float _height;
    BottomPanelTab _active_tab;
};

}
