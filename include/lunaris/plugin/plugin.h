#pragma once

#include <cstdint>

namespace lunaris {

using PluginID = uint32_t;

constexpr PluginID INVALID_PLUGIN_ID = 0;

enum class PluginCategory : uint8_t {
    Core,
    Language,
    Theme,
    Tool,
    Integration,
    Other
};

struct PluginInfo {
    const char* name;
    const char* version;
    const char* author;
    const char* description;
    PluginCategory category;
};

class EditorContext;

class Plugin {
public:
    Plugin() : _id(INVALID_PLUGIN_ID), _enabled(false) {}
    virtual ~Plugin() = default;

    virtual PluginInfo get_info() const = 0;

    virtual void on_load(EditorContext* context) { (void)context; }
    virtual void on_unload() {}
    virtual void on_enable() {}
    virtual void on_disable() {}

    virtual void on_update(float delta_time) { (void)delta_time; }
    virtual void on_ui() {}
    virtual void on_menu_bar() {}
    virtual void on_toolbar() {}
    virtual void on_status_bar() {}

    virtual void on_file_opened(const char* path) { (void)path; }
    virtual void on_file_saved(const char* path) { (void)path; }
    virtual void on_file_closed(const char* path) { (void)path; }

    virtual void on_workspace_ui() {}

    PluginID get_id() const { return _id; }
    bool is_enabled() const { return _enabled; }

protected:
    EditorContext* get_context() const { return _context; }

private:
    friend class PluginManager;

    void set_id(PluginID id) { _id = id; }
    void set_enabled(bool enabled) { _enabled = enabled; }
    void set_context(EditorContext* ctx) { _context = ctx; }

    PluginID _id;
    bool _enabled;
    EditorContext* _context;
};

}
