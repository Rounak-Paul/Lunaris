#pragma once

#include <cstdint>

namespace lunaris {

class Theme;

using TabID = uint32_t;
constexpr TabID INVALID_TAB_ID = 0;

struct TabInfo {
    const char* title;
    const char* filepath;
    bool modified;
    bool pinned;
};

class TabBar {
public:
    static constexpr uint32_t MAX_TABS = 64;
    static constexpr float TAB_HEIGHT = 32.0f;
    static constexpr float TAB_MIN_WIDTH = 80.0f;
    static constexpr float TAB_MAX_WIDTH = 200.0f;

    TabBar();
    ~TabBar();

    void set_theme(Theme* theme) { _theme = theme; }

    void on_init();
    void on_shutdown();
    void on_ui();

    TabID add_tab(const TabInfo& info);
    void remove_tab(TabID id);
    void set_active_tab(TabID id);
    TabID get_active_tab() const { return _active_tab; }

    void set_tab_modified(TabID id, bool modified);
    void set_tab_title(TabID id, const char* title);

    uint32_t get_tab_count() const { return _tab_count; }
    bool has_tabs() const { return _tab_count > 0; }

    float get_height() const { return _tab_count > 0 ? TAB_HEIGHT : 0.0f; }

private:
    struct Tab {
        TabID id;
        TabInfo info;
    };

    TabID generate_id();
    void draw_tab(uint32_t index);

    Tab _tabs[MAX_TABS];
    uint32_t _tab_count;
    TabID _active_tab;
    TabID _next_id;
    Theme* _theme;
};

}
