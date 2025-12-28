#pragma once

namespace lunaris {

class Theme;
class PluginManager;
class CommandRegistry;

class MenuBar {
public:
    MenuBar();
    ~MenuBar();

    void set_theme(Theme* theme) { _theme = theme; }
    void set_plugin_manager(PluginManager* manager) { _plugin_manager = manager; }
    void set_command_registry(CommandRegistry* registry) { _command_registry = registry; }

    void on_init();
    void on_shutdown();
    void on_ui();

private:
    void draw_file_menu();
    void draw_edit_menu();
    void draw_view_menu();
    void draw_build_menu();
    void draw_debug_menu();
    void draw_plugins_menu();
    void draw_help_menu();

    Theme* _theme;
    PluginManager* _plugin_manager;
    CommandRegistry* _command_registry;
};

}
