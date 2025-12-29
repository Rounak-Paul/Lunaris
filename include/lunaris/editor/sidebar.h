#pragma once

#include <cstdint>

namespace lunaris {

class Theme;
class PluginManager;
class FileTree;
class FileOperations;

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
    static constexpr float DEFAULT_CONTENT_WIDTH = 200.0f;
    static constexpr float MIN_CONTENT_WIDTH = 120.0f;
    static constexpr float MAX_CONTENT_WIDTH = 500.0f;

    Sidebar();
    ~Sidebar();

    void set_theme(Theme* theme);
    void set_plugin_manager(PluginManager* manager) { _plugin_manager = manager; }
    void set_file_operations(FileOperations* ops);

    void on_init();
    void on_shutdown();
    void on_ui();

    void toggle() { _panel_visible = !_panel_visible; }
    void show() { _panel_visible = true; }
    void hide() { _panel_visible = false; }
    bool is_panel_visible() const { return _panel_visible; }

    void set_active_panel(SidebarPanel panel) { _active_panel = panel; }
    SidebarPanel get_active_panel() const { return _active_panel; }

    float get_width() const { return ICON_BAR_WIDTH + (_panel_visible ? _content_width : 0.0f); }
    float get_content_width() const { return _content_width; }
    void set_content_width(float w);

    void open_folder();
    void open_folder(const char* path);
    void close_folder();
    bool has_folder() const;
    const char* get_folder_path() const;

    void refresh_file_tree();

    using FileSelectedCallback = void(*)(const char* path, void* user_data);
    void set_file_selected_callback(FileSelectedCallback cb, void* user_data);

private:
    void draw_panel_tabs();
    void draw_explorer();
    void draw_search();
    void draw_resize_handle(float sidebar_start_x, float sidebar_start_y, float total_height);

    Theme* _theme;
    PluginManager* _plugin_manager;
    FileTree* _file_tree;
    bool _panel_visible;
    bool _is_resizing;
    SidebarPanel _active_panel;
    float _content_width;
    char _search_buffer[256];
};

}
