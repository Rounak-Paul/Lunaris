#pragma once

#include "lunaris/plugin/plugin.h"

namespace lunaris {

struct LoadedPlugin {
    void* handle;
    Plugin* plugin;
    void (*destroy_func)(Plugin*);
};

class PluginLoader {
public:
    static constexpr uint32_t MAX_LOADED_PLUGINS = 64;

    PluginLoader();
    ~PluginLoader();

    bool load_plugin(const char* path, LoadedPlugin& out);
    void unload_plugin(LoadedPlugin& loaded);

    uint32_t load_plugins_from_directory(const char* directory, LoadedPlugin* out_plugins, uint32_t max_count);

private:
    static const char* get_plugin_extension();
};

}
