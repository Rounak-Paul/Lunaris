#pragma once

namespace lunaris {

class PluginManager;

class StatusBar {
public:
    StatusBar();
    ~StatusBar();

    void set_plugin_manager(PluginManager* manager) { _plugin_manager = manager; }

    void on_init();
    void on_shutdown();
    void on_ui();

    void set_status_text(const char* text);
    void set_repo_name(const char* name);
    void set_branch_name(const char* name);
    void set_build_status(const char* status);
    void set_progress(float progress, const char* label = nullptr);
    void clear_progress();

    const char* get_status_text() const { return _status_text; }
    const char* get_repo_name() const { return _repo_name; }
    const char* get_branch_name() const { return _branch_name; }

private:
    void draw_left_section();
    void draw_center_section();
    void draw_right_section();

    PluginManager* _plugin_manager;

    char _status_text[256];
    char _repo_name[128];
    char _branch_name[64];
    char _build_status[64];
    char _progress_label[128];
    float _progress;
    bool _show_progress;
};

}
