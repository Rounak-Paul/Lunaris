#include "lunaris/plugin/plugin_loader.h"
#include "lunaris/plugin/plugin_api.h"
#include <cstring>
#include <cstdio>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
    #include <dirent.h>
#endif

namespace lunaris {

PluginLoader::PluginLoader() {
}

PluginLoader::~PluginLoader() {
}

const char* PluginLoader::get_plugin_extension() {
#ifdef _WIN32
    return ".dll";
#elif __APPLE__
    return ".dylib";
#else
    return ".so";
#endif
}

bool PluginLoader::load_plugin(const char* path, LoadedPlugin& out) {
    out.handle = nullptr;
    out.plugin = nullptr;
    out.destroy_func = nullptr;

#ifdef _WIN32
    HMODULE handle = LoadLibraryA(path);
    if (!handle) {
        return false;
    }

    auto get_version = (uint32_t(*)())GetProcAddress(handle, "lunaris_get_api_version");
    auto create_func = (Plugin*(*)())GetProcAddress(handle, "lunaris_create_plugin");
    auto destroy_func = (void(*)(Plugin*))GetProcAddress(handle, "lunaris_destroy_plugin");
#else
    void* handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        return false;
    }

    auto get_version = (uint32_t(*)())dlsym(handle, "lunaris_get_api_version");
    auto create_func = (Plugin*(*)())dlsym(handle, "lunaris_create_plugin");
    auto destroy_func = (void(*)(Plugin*))dlsym(handle, "lunaris_destroy_plugin");
#endif

    if (!get_version || !create_func || !destroy_func) {
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return false;
    }

    uint32_t version = get_version();
    if (version != LUNARIS_PLUGIN_API_VERSION) {
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return false;
    }

    Plugin* plugin = create_func();
    if (!plugin) {
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return false;
    }

    out.handle = handle;
    out.plugin = plugin;
    out.destroy_func = destroy_func;

    return true;
}

void PluginLoader::unload_plugin(LoadedPlugin& loaded) {
    if (loaded.plugin && loaded.destroy_func) {
        loaded.destroy_func(loaded.plugin);
    }

    if (loaded.handle) {
#ifdef _WIN32
        FreeLibrary((HMODULE)loaded.handle);
#else
        dlclose(loaded.handle);
#endif
    }

    loaded.handle = nullptr;
    loaded.plugin = nullptr;
    loaded.destroy_func = nullptr;
}

uint32_t PluginLoader::load_plugins_from_directory(const char* directory, LoadedPlugin* out_plugins, uint32_t max_count) {
    uint32_t count = 0;
    const char* ext = get_plugin_extension();
    size_t ext_len = strlen(ext);

#ifdef _WIN32
    char search_path[512];
    snprintf(search_path, sizeof(search_path), "%s\\*%s", directory, ext);

    WIN32_FIND_DATAA find_data;
    HANDLE find_handle = FindFirstFileA(search_path, &find_data);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return 0;
    }

    do {
        if (count >= max_count) {
            break;
        }

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s\\%s", directory, find_data.cFileName);

        if (load_plugin(full_path, out_plugins[count])) {
            count++;
        }
    } while (FindNextFileA(find_handle, &find_data));

    FindClose(find_handle);
#else
    DIR* dir = opendir(directory);
    if (!dir) {
        return 0;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (count >= max_count) {
            break;
        }

        size_t name_len = strlen(entry->d_name);
        if (name_len <= ext_len) {
            continue;
        }

        if (strcmp(entry->d_name + name_len - ext_len, ext) != 0) {
            continue;
        }

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", directory, entry->d_name);

        if (load_plugin(full_path, out_plugins[count])) {
            count++;
        }
    }

    closedir(dir);
#endif

    return count;
}

}
