#pragma once

#include "lunaris/plugin/plugin.h"

#ifdef _WIN32
    #define LUNARIS_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
    #define LUNARIS_PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
#endif

#define LUNARIS_PLUGIN_API_VERSION 1

struct PluginAPI {
    uint32_t api_version;
    lunaris::Plugin* (*create_plugin)();
    void (*destroy_plugin)(lunaris::Plugin*);
};

#define LUNARIS_DECLARE_PLUGIN(PluginClass) \
    LUNARIS_PLUGIN_EXPORT lunaris::Plugin* lunaris_create_plugin() { \
        return new PluginClass(); \
    } \
    LUNARIS_PLUGIN_EXPORT void lunaris_destroy_plugin(lunaris::Plugin* plugin) { \
        delete plugin; \
    } \
    LUNARIS_PLUGIN_EXPORT uint32_t lunaris_get_api_version() { \
        return LUNARIS_PLUGIN_API_VERSION; \
    }
