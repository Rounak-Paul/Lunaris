#pragma once

namespace lunaris {

class Workspace;
class PluginManager;
class EditorContext;
class JobSystem;
class StatusBar;
class ActivityBar;
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
    ActivityBar* get_activity_bar() const { return _activity_bar; }
    Theme* get_theme() const { return _theme; }

private:
    void setup_dockspace();
    void draw_activity_bar();
    void draw_workspace();
    void draw_status_bar();

    Workspace* _workspace;
    StatusBar* _status_bar;
    ActivityBar* _activity_bar;
    PluginManager* _plugin_manager;
    EditorContext* _context;
    JobSystem* _job_system;
    Theme* _theme;
    bool _show_demo_window;
    bool _first_frame;
};

}
