#pragma once

#include "lunaris/plugin/plugin.h"
#include "lunaris/plugin/editor_context.h"

namespace lunaris {

class PluginManager {
public:
    static constexpr uint32_t MAX_PLUGINS = 128;

    PluginManager();
    ~PluginManager();

    void set_context(EditorContext* context);

    PluginID register_plugin(Plugin* plugin);
    void unregister_plugin(PluginID id);
    void unregister_all();

    void enable_plugin(PluginID id);
    void disable_plugin(PluginID id);

    Plugin* get_plugin(PluginID id) const;
    Plugin* find_plugin_by_name(const char* name) const;
    uint32_t get_plugin_count() const { return _plugin_count; }

    void update_all(float delta_time);
    void ui_all();
    void menu_bar_all();
    void toolbar_all();
    void status_bar_all();
    void workspace_ui_all();

    void notify_file_opened(const char* path);
    void notify_file_saved(const char* path);
    void notify_file_closed(const char* path);

private:
    PluginID generate_id();

    Plugin* _plugins[MAX_PLUGINS];
    uint32_t _plugin_count;
    PluginID _next_id;
    EditorContext* _context;
};

}
