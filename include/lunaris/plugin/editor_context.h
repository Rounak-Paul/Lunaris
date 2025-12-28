#pragma once

namespace lunaris {

class Workspace;
class PluginManager;
class JobSystem;

class EditorContext {
public:
    EditorContext() 
        : _workspace(nullptr)
        , _plugin_manager(nullptr)
        , _job_system(nullptr) {}

    void set_workspace(Workspace* workspace) { _workspace = workspace; }
    void set_plugin_manager(PluginManager* manager) { _plugin_manager = manager; }
    void set_job_system(JobSystem* job_system) { _job_system = job_system; }

    Workspace* get_workspace() const { return _workspace; }
    PluginManager* get_plugin_manager() const { return _plugin_manager; }
    JobSystem* get_job_system() const { return _job_system; }

private:
    Workspace* _workspace;
    PluginManager* _plugin_manager;
    JobSystem* _job_system;
};

}
