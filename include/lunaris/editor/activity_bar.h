#pragma once

#include <cstdint>

namespace lunaris {

class PluginManager;
class Theme;

using ActivityID = uint32_t;
constexpr ActivityID INVALID_ACTIVITY_ID = 0;

enum class BuiltinActivity : uint8_t {
    Explorer = 1,
    Search = 2,
    SourceControl = 3,
    Debug = 4,
    Extensions = 5
};

struct ActivityInfo {
    const char* name;
    const char* icon;
    const char* tooltip;
};

class ActivityBar {
public:
    static constexpr uint32_t MAX_ACTIVITIES = 32;
    static constexpr float BAR_WIDTH = 48.0f;
    static constexpr float ICON_SIZE = 24.0f;

    ActivityBar();
    ~ActivityBar();

    void set_plugin_manager(PluginManager* manager) { _plugin_manager = manager; }
    void set_theme(Theme* theme) { _theme = theme; }

    void on_init();
    void on_shutdown();
    void on_ui();

    ActivityID register_activity(const ActivityInfo& info);
    void unregister_activity(ActivityID id);

    void set_active(ActivityID id);
    ActivityID get_active() const { return _active_id; }

    bool is_activity_active(ActivityID id) const { return _active_id == id; }

private:
    void draw_activity_button(uint32_t index);
    ActivityID generate_id();

    struct Activity {
        ActivityID id;
        ActivityInfo info;
        bool from_plugin;
    };

    Activity _activities[MAX_ACTIVITIES];
    uint32_t _activity_count;
    ActivityID _active_id;
    ActivityID _next_id;

    PluginManager* _plugin_manager;
    Theme* _theme;
};

}
