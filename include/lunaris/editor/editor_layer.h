#pragma once

namespace lunaris {

class Workspace;
class PluginManager;
class EditorContext;
class JobSystem;

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

private:
    void setup_dockspace();
    void draw_menu_bar();
    void draw_workspace();

    Workspace* _workspace;
    PluginManager* _plugin_manager;
    EditorContext* _context;
    JobSystem* _job_system;
    bool _show_demo_window;
    bool _first_frame;
};

}
