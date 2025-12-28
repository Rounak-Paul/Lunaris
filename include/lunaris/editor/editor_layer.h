#pragma once

namespace lunaris {

class Workspace;
class PluginManager;
class EditorContext;
class JobSystem;
class StatusBar;
class CommandRegistry;
class CommandPalette;
class MenuBar;
class Sidebar;
class TabBar;
class BottomPanel;
class Theme;

class EditorLayer {
public:
    EditorLayer();
    ~EditorLayer();

    void on_init();
    void on_shutdown();
    void on_update(float delta_time);
    void on_ui();

    PluginManager* get_plugin_manager() const { return _plugin_manager; }
    EditorContext* get_context() const { return _context; }
    JobSystem* get_job_system() const { return _job_system; }
    StatusBar* get_status_bar() const { return _status_bar; }
    CommandRegistry* get_command_registry() const { return _command_registry; }
    CommandPalette* get_command_palette() const { return _command_palette; }
    Sidebar* get_sidebar() const { return _sidebar; }
    BottomPanel* get_bottom_panel() const { return _bottom_panel; }
    Theme* get_theme() const { return _theme; }

private:
    void setup_layout();
    void draw_main_area();
    void register_builtin_commands();
    void handle_keyboard_shortcuts();

    Workspace* _workspace;
    StatusBar* _status_bar;
    MenuBar* _menu_bar;
    Sidebar* _sidebar;
    TabBar* _tab_bar;
    BottomPanel* _bottom_panel;
    CommandRegistry* _command_registry;
    CommandPalette* _command_palette;
    PluginManager* _plugin_manager;
    EditorContext* _context;
    JobSystem* _job_system;
    Theme* _theme;
    bool _first_frame;
};

}
