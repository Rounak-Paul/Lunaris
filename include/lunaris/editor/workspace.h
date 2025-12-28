#pragma once

namespace lunaris {

class PluginManager;

class Workspace {
public:
    Workspace();
    ~Workspace();

    void set_plugin_manager(PluginManager* manager) { _plugin_manager = manager; }

    void on_init();
    void on_shutdown();
    void on_update(float delta_time);
    void on_ui();

private:
    PluginManager* _plugin_manager;
};

}
