#pragma once

#include "lunaris/plugin/plugin.h"

namespace lunaris {

class Theme;

enum class ThemePreset : uint8_t {
    Default,
    Minimalist,
    Retro,
    Count
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

    static const char* get_preset_name(ThemePreset preset);
    static ThemePlugin* instance() { return s_instance; }

private:
    void register_commands();
    void apply_default(Theme* theme);
    void apply_minimalist(Theme* theme);
    void apply_retro(Theme* theme);

    ThemePreset _current_preset;
    static ThemePlugin* s_instance;
};

}
