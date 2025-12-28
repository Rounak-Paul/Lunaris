#pragma once

#include <cstdint>

namespace lunaris {

class Theme;
class PluginManager;

enum class SidebarPanel : uint8_t {
    Explorer,
    Search,
    SourceControl,
    Debug,
    Extensions
};

class Sidebar {
public:
    static constexpr float ICON_BAR_WIDTH = 40.0f;
    static constexpr float CONTENT_WIDTH = 200.0f;
    static constexpr float MIN_WIDTH = 180.0f;
    static constexpr float MAX_WIDTH = 400.0f;

    Sidebar();
    ~Sidebar();

    void set_theme(Theme* theme) { _theme = theme; }
    void set_plugin_manager(PluginManager* manager) { _plugin_manager = manager; }

    void on_init();
    void on_shutdown();
    void on_ui();

    void toggle() { _panel_visible = !_panel_visible; }
    void show() { _panel_visible = true; }
    void hide() { _panel_visible = false; }
    bool is_panel_visible() const { return _panel_visible; }

    void set_active_panel(SidebarPanel panel) { _active_panel = panel; }
    SidebarPanel get_active_panel() const { return _active_panel; }

    float get_width() const { return ICON_BAR_WIDTH + (_panel_visible ? CONTENT_WIDTH : 0.0f); }

private:
    void draw_panel_tabs();
    void draw_explorer();
    void draw_search();

    Theme* _theme;
    PluginManager* _plugin_manager;
    bool _panel_visible;
    SidebarPanel _active_panel;
    char _search_buffer[256];
};

}
