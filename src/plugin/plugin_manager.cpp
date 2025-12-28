#include "lunaris/plugin/plugin_manager.h"
#include <cstring>
#include <cstdio>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <libgen.h>
#endif

namespace lunaris {

static void get_executable_directory(char* buffer, uint32_t size) {
#ifdef __APPLE__
    uint32_t buf_size = size;
    if (_NSGetExecutablePath(buffer, &buf_size) == 0) {
        char* dir = dirname(buffer);
        strncpy(buffer, dir, size - 1);
        buffer[size - 1] = '\0';
    } else {
        buffer[0] = '.';
        buffer[1] = '\0';
    }
#elif defined(_WIN32)
    GetModuleFileNameA(nullptr, buffer, size);
    char* last_slash = strrchr(buffer, '\\');
    if (last_slash) {
        *last_slash = '\0';
    }
#else
    ssize_t len = readlink("/proc/self/exe", buffer, size - 1);
    if (len != -1) {
        buffer[len] = '\0';
        char* last_slash = strrchr(buffer, '/');
        if (last_slash) {
            *last_slash = '\0';
        }
    } else {
        buffer[0] = '.';
        buffer[1] = '\0';
    }
#endif
}

PluginManager::PluginManager()
    : _loaded_plugin_count(0)
    , _plugin_count(0)
    , _next_id(1)
    , _context(nullptr) {
    for (uint32_t i = 0; i < MAX_PLUGINS; ++i) {
        _plugins[i] = nullptr;
    }
    for (uint32_t i = 0; i < PluginLoader::MAX_LOADED_PLUGINS; ++i) {
        _loaded_plugins[i].handle = nullptr;
        _loaded_plugins[i].plugin = nullptr;
        _loaded_plugins[i].destroy_func = nullptr;
    }
}

PluginManager::~PluginManager() {
    unregister_all();
    for (uint32_t i = 0; i < _loaded_plugin_count; ++i) {
        _loader.unload_plugin(_loaded_plugins[i]);
    }
    _loaded_plugin_count = 0;
}

void PluginManager::set_context(EditorContext* context) {
    _context = context;
}

void PluginManager::load_plugins_from_directory(const char* directory) {
    char exe_dir[512];
    get_executable_directory(exe_dir, sizeof(exe_dir));
    
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", exe_dir, directory);
    
    LoadedPlugin temp_plugins[PluginLoader::MAX_LOADED_PLUGINS];
    uint32_t loaded = _loader.load_plugins_from_directory(full_path, temp_plugins, PluginLoader::MAX_LOADED_PLUGINS);

    for (uint32_t i = 0; i < loaded; ++i) {
        if (_loaded_plugin_count >= PluginLoader::MAX_LOADED_PLUGINS) {
            _loader.unload_plugin(temp_plugins[i]);
            continue;
        }

        PluginID id = register_plugin(temp_plugins[i].plugin);
        if (id != INVALID_PLUGIN_ID) {
            _loaded_plugins[_loaded_plugin_count] = temp_plugins[i];
            _loaded_plugin_count++;
            enable_plugin(id);
        } else {
            _loader.unload_plugin(temp_plugins[i]);
        }
    }
}

PluginID PluginManager::generate_id() {
    return _next_id++;
}

PluginID PluginManager::register_plugin(Plugin* plugin) {
    if (!plugin || _plugin_count >= MAX_PLUGINS) {
        return INVALID_PLUGIN_ID;
    }

    for (uint32_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_plugins[i] == nullptr) {
            PluginID id = generate_id();
            plugin->set_id(id);
            plugin->set_context(_context);
            plugin->on_load(_context);
            _plugins[i] = plugin;
            _plugin_count++;
            return id;
        }
    }

    return INVALID_PLUGIN_ID;
}

void PluginManager::unregister_plugin(PluginID id) {
    for (uint32_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_plugins[i] && _plugins[i]->get_id() == id) {
            if (_plugins[i]->is_enabled()) {
                _plugins[i]->on_disable();
            }
            _plugins[i]->on_unload();
            _plugins[i] = nullptr;
            _plugin_count--;
            return;
        }
    }
}

void PluginManager::unregister_all() {
    for (uint32_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_plugins[i]) {
            if (_plugins[i]->is_enabled()) {
                _plugins[i]->on_disable();
            }
            _plugins[i]->on_unload();
            _plugins[i] = nullptr;
        }
    }
    _plugin_count = 0;
}

void PluginManager::enable_plugin(PluginID id) {
    Plugin* plugin = get_plugin(id);
    if (plugin && !plugin->is_enabled()) {
        plugin->set_enabled(true);
        plugin->on_enable();
    }
}

void PluginManager::disable_plugin(PluginID id) {
    Plugin* plugin = get_plugin(id);
    if (plugin && plugin->is_enabled()) {
        plugin->on_disable();
        plugin->set_enabled(false);
    }
}

Plugin* PluginManager::get_plugin(PluginID id) const {
    for (uint32_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_plugins[i] && _plugins[i]->get_id() == id) {
            return _plugins[i];
        }
    }
    return nullptr;
}

Plugin* PluginManager::find_plugin_by_name(const char* name) const {
    for (uint32_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_plugins[i]) {
            PluginInfo info = _plugins[i]->get_info();
            if (strcmp(info.name, name) == 0) {
                return _plugins[i];
            }
        }
    }
    return nullptr;
}

void PluginManager::update_all(float delta_time) {
    for (uint32_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_plugins[i] && _plugins[i]->is_enabled()) {
            _plugins[i]->on_update(delta_time);
        }
    }
}

void PluginManager::ui_all() {
    for (uint32_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_plugins[i] && _plugins[i]->is_enabled()) {
            _plugins[i]->on_ui();
        }
    }
}

void PluginManager::menu_bar_all() {
    for (uint32_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_plugins[i] && _plugins[i]->is_enabled()) {
            _plugins[i]->on_menu_bar();
        }
    }
}

void PluginManager::toolbar_all() {
    for (uint32_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_plugins[i] && _plugins[i]->is_enabled()) {
            _plugins[i]->on_toolbar();
        }
    }
}

void PluginManager::status_bar_all() {
    for (uint32_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_plugins[i] && _plugins[i]->is_enabled()) {
            _plugins[i]->on_status_bar();
        }
    }
}

void PluginManager::workspace_ui_all() {
    for (uint32_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_plugins[i] && _plugins[i]->is_enabled()) {
            _plugins[i]->on_workspace_ui();
        }
    }
}

void PluginManager::notify_file_opened(const char* path) {
    for (uint32_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_plugins[i] && _plugins[i]->is_enabled()) {
            _plugins[i]->on_file_opened(path);
        }
    }
}

void PluginManager::notify_file_saved(const char* path) {
    for (uint32_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_plugins[i] && _plugins[i]->is_enabled()) {
            _plugins[i]->on_file_saved(path);
        }
    }
}

void PluginManager::notify_file_closed(const char* path) {
    for (uint32_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_plugins[i] && _plugins[i]->is_enabled()) {
            _plugins[i]->on_file_closed(path);
        }
    }
}

}
