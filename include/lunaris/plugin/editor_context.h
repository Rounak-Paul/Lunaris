#pragma once

namespace lunaris {

class Workspace;
class PluginManager;
class JobSystem;
class Theme;
class CommandRegistry;

class EditorContext {
public:
    EditorContext() 
        : _workspace(nullptr)
        , _plugin_manager(nullptr)
        , _job_system(nullptr)
        , _theme(nullptr)
        , _command_registry(nullptr) {}

    void set_workspace(Workspace* workspace) { _workspace = workspace; }
    void set_plugin_manager(PluginManager* manager) { _plugin_manager = manager; }
    void set_job_system(JobSystem* job_system) { _job_system = job_system; }
    void set_theme(Theme* theme) { _theme = theme; }
    void set_command_registry(CommandRegistry* registry) { _command_registry = registry; }

    Workspace* get_workspace() const { return _workspace; }
    PluginManager* get_plugin_manager() const { return _plugin_manager; }
    JobSystem* get_job_system() const { return _job_system; }
    Theme* get_theme() const { return _theme; }
    CommandRegistry* get_command_registry() const { return _command_registry; }

private:
    Workspace* _workspace;
    PluginManager* _plugin_manager;
    JobSystem* _job_system;
    Theme* _theme;
    CommandRegistry* _command_registry;
};

}
