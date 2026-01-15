#pragma once

#include "lunaris/plugin/plugin.h"
#include "lunaris/core/theme.h"

namespace lunaris {

enum class ThemePreset : uint8_t {
    Retro,
    DarkMinimalist
};

class ThemePlugin : public Plugin {
public:
    ThemePlugin();
    ~ThemePlugin() override;

    PluginInfo get_info() const override;

    void on_load(EditorContext* context) override;
    void on_unload() override;
    void on_enable() override;

    void set_preset(ThemePreset preset);
    ThemePreset get_current_preset() const { return _current_preset; }

    static ThemeConfig make_retro();
    static ThemeConfig make_dark_minimalist();

    static const char* get_preset_name(ThemePreset preset);
    static ThemePlugin* instance() { return s_instance; }

private:
    void register_commands();

    ThemePreset _current_preset;
    static ThemePlugin* s_instance;
};

}
