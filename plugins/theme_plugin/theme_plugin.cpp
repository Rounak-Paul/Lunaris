#include "theme_plugin.h"
#include "lunaris/plugin/plugin_api.h"
#include "lunaris/plugin/editor_context.h"
#include "lunaris/core/command_registry.h"
#include "lunaris/core/theme.h"

namespace lunaris {

ThemePlugin* ThemePlugin::s_instance = nullptr;

ThemePlugin::ThemePlugin()
    : _current_preset(ThemePreset::Retro) {
    s_instance = this;
}

ThemePlugin::~ThemePlugin() {
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

PluginInfo ThemePlugin::get_info() const {
    PluginInfo info;
    info.name = "Theme";
    info.version = "1.0.0";
    info.author = "Lunaris";
    info.description = "Built-in theme switcher with Retro and Dark Minimalist presets";
    info.category = PluginCategory::Theme;
    return info;
}

void ThemePlugin::on_load(EditorContext* context) {
    (void)context;
    register_commands();
}

void ThemePlugin::on_unload() {
}

void ThemePlugin::on_enable() {
    set_preset(_current_preset);
}

void ThemePlugin::set_preset(ThemePreset preset) {
    _current_preset = preset;
    Theme* theme = get_context()->get_theme();
    if (!theme) {
        return;
    }
    
    ThemeConfig cfg;
    const char* name = nullptr;
    
    switch (preset) {
        case ThemePreset::Retro:
            cfg = make_retro();
            name = "Retro";
            break;
        case ThemePreset::DarkMinimalist:
            cfg = make_dark_minimalist();
            name = "Dark Minimalist";
            break;
    }
    
    theme->set_config(cfg);
    theme->set_active_name(name);
    theme->apply();
}

const char* ThemePlugin::get_preset_name(ThemePreset preset) {
    switch (preset) {
        case ThemePreset::Retro: return "Retro";
        case ThemePreset::DarkMinimalist: return "Dark Minimalist";
        default: return "Unknown";
    }
}

ThemeConfig ThemePlugin::make_retro() {
    ThemeConfig cfg;
    ColorPalette& p = cfg.palette;
    
    Color light_edge = Color::rgb(60, 75, 60);
    Color dark_edge = Color::rgb(4, 6, 4);
    Color face = Color::rgb(20, 24, 20);
    Color face_hover = Color::rgb(28, 36, 28);
    Color face_dark = Color::rgb(12, 16, 12);
    Color phosphor = Color::rgb(0, 255, 65);
    Color phosphor_dim = Color::rgb(0, 200, 50);
    Color phosphor_dark = Color::rgb(0, 120, 30);
    Color screen = Color::rgb(0, 8, 2);
    Color screen_glow = Color::rgb(0, 16, 4);
    
    p.background = face;
    p.background_alt = face_dark;
    p.background_highlight = Color::rgb(0, 40, 10);
    
    p.foreground = phosphor;
    p.foreground_dim = phosphor_dim;
    p.foreground_disabled = phosphor_dark;
    
    p.border = dark_edge;
    p.border_light = light_edge;
    p.border_focus = phosphor;
    
    p.accent = phosphor;
    p.accent_hover = Color::rgb(50, 255, 100);
    p.accent_active = phosphor_dim;
    
    p.success = phosphor;
    p.warning = Color::rgb(255, 200, 0);
    p.error = Color::rgb(255, 50, 50);
    p.info = phosphor_dim;
    
    p.selection = Color::rgb(0, 255, 65, 100);
    p.shadow = Color::rgb(0, 0, 0, 240);

    cfg.button.background = face;
    cfg.button.background_hover = face_hover;
    cfg.button.background_active = face_dark;
    cfg.button.background_disabled = face_dark;
    cfg.button.foreground = phosphor;
    cfg.button.foreground_hover = Color::rgb(100, 255, 130);
    cfg.button.foreground_active = phosphor_dim;
    cfg.button.foreground_disabled = phosphor_dark;
    cfg.button.border = dark_edge;
    cfg.button.border_hover = dark_edge;
    cfg.button.border_active = dark_edge;
    cfg.button.border_disabled = p.foreground_disabled;
    cfg.button.border_thickness = 2.0f;
    cfg.button.rounding = 0.0f;
    cfg.button.padding_x = 12.0f;
    cfg.button.padding_y = 4.0f;
    cfg.button.border_style = BorderStyle::Raised;
    
    cfg.checkbox = cfg.button;
    cfg.checkbox.background = screen;
    cfg.checkbox.background_hover = screen_glow;
    cfg.checkbox.background_active = screen;
    cfg.checkbox.padding_x = 2.0f;
    cfg.checkbox.padding_y = 2.0f;
    cfg.checkbox.border_style = BorderStyle::Sunken;
    
    cfg.radio = cfg.checkbox;
    cfg.radio.border_style = BorderStyle::Sunken;
    
    cfg.slider = cfg.button;
    cfg.slider.background = screen;
    cfg.slider.foreground = face;
    cfg.slider.foreground_hover = face_hover;
    cfg.slider.foreground_active = face_dark;
    cfg.slider.padding_x = 0.0f;
    cfg.slider.padding_y = 0.0f;
    cfg.slider.border_style = BorderStyle::Sunken;
    
    cfg.input.background = screen;
    cfg.input.background_hover = screen_glow;
    cfg.input.background_focused = screen;
    cfg.input.foreground = phosphor;
    cfg.input.placeholder = phosphor_dark;
    cfg.input.selection = Color::rgb(0, 255, 65, 100);
    cfg.input.cursor = phosphor;
    cfg.input.border = dark_edge;
    cfg.input.border_hover = light_edge;
    cfg.input.border_focused = phosphor;
    cfg.input.border_thickness = 2.0f;
    cfg.input.rounding = 0.0f;
    cfg.input.padding_x = 6.0f;
    cfg.input.padding_y = 4.0f;
    cfg.input.border_style = BorderStyle::Sunken;
    
    cfg.window.background = face;
    cfg.window.title_background = face_dark;
    cfg.window.title_background_active = Color::rgb(0, 80, 20);
    cfg.window.title_foreground = phosphor;
    cfg.window.border = dark_edge;
    cfg.window.border_thickness = 3.0f;
    cfg.window.rounding = 0.0f;
    cfg.window.padding = 6.0f;
    cfg.window.title_height = 22.0f;
    cfg.window.border_style = BorderStyle::Raised;
    
    cfg.scrollbar.track = screen;
    cfg.scrollbar.thumb = face;
    cfg.scrollbar.thumb_hover = face_hover;
    cfg.scrollbar.thumb_active = face_dark;
    cfg.scrollbar.width = 18.0f;
    cfg.scrollbar.rounding = 0.0f;
    cfg.scrollbar.min_thumb_size = 24.0f;
    
    cfg.menu.background = face;
    cfg.menu.item_hover = Color::rgb(0, 80, 20);
    cfg.menu.item_active = Color::rgb(0, 100, 25);
    cfg.menu.foreground = phosphor;
    cfg.menu.foreground_hover = Color::rgb(100, 255, 130);
    cfg.menu.foreground_disabled = phosphor_dark;
    cfg.menu.separator = light_edge;
    cfg.menu.border = dark_edge;
    cfg.menu.border_thickness = 2.0f;
    cfg.menu.rounding = 0.0f;
    cfg.menu.item_padding_x = 16.0f;
    cfg.menu.item_padding_y = 6.0f;
    
    cfg.tab.background = face_dark;
    cfg.tab.background_hover = face;
    cfg.tab.background_active = face;
    cfg.tab.foreground = phosphor_dim;
    cfg.tab.foreground_active = phosphor;
    cfg.tab.border = dark_edge;
    cfg.tab.underline = phosphor;
    cfg.tab.rounding = 0.0f;
    cfg.tab.padding_x = 14.0f;
    cfg.tab.padding_y = 6.0f;
    
    cfg.font_size = 14.0f;
    cfg.item_spacing_x = 6.0f;
    cfg.item_spacing_y = 6.0f;
    cfg.indent_spacing = 18.0f;
    cfg.pixel_perfect = true;

    return cfg;
}

ThemeConfig ThemePlugin::make_dark_minimalist() {
    ThemeConfig cfg;
    ColorPalette& p = cfg.palette;
    
    p.background = Color::rgb(13, 13, 13);
    p.background_alt = Color::rgb(18, 18, 18);
    p.background_highlight = Color::rgb(26, 26, 26);
    
    p.foreground = Color::rgb(217, 217, 217);
    p.foreground_dim = Color::rgb(115, 115, 115);
    p.foreground_disabled = Color::rgb(64, 64, 64);
    
    p.border = Color::rgb(38, 38, 38);
    p.border_light = Color::rgb(51, 51, 51);
    p.border_focus = Color::rgb(153, 153, 153);
    
    p.accent = Color::rgb(153, 153, 153);
    p.accent_hover = Color::rgb(191, 191, 191);
    p.accent_active = Color::rgb(128, 128, 128);
    
    p.success = Color::rgb(128, 179, 128);
    p.warning = Color::rgb(204, 179, 102);
    p.error = Color::rgb(204, 102, 102);
    p.info = Color::rgb(102, 153, 204);
    
    p.selection = Color::rgb(153, 153, 153, 51);
    p.shadow = Color::rgb(0, 0, 0, 128);

    cfg.button.background = p.background_alt;
    cfg.button.background_hover = p.background_highlight;
    cfg.button.background_active = p.accent;
    cfg.button.background_disabled = p.background;
    cfg.button.foreground = p.foreground;
    cfg.button.foreground_hover = p.foreground;
    cfg.button.foreground_active = p.background;
    cfg.button.foreground_disabled = p.foreground_disabled;
    cfg.button.border = p.border;
    cfg.button.border_hover = p.border_light;
    cfg.button.border_active = p.accent;
    cfg.button.border_disabled = p.border;
    cfg.button.border_thickness = 0.0f;
    cfg.button.rounding = 3.0f;
    cfg.button.padding_x = 8.0f;
    cfg.button.padding_y = 4.0f;
    cfg.button.border_style = BorderStyle::None;
    
    cfg.checkbox = cfg.button;
    cfg.checkbox.background = p.background;
    cfg.checkbox.background_active = p.accent;
    cfg.checkbox.padding_x = 2.0f;
    cfg.checkbox.padding_y = 2.0f;
    cfg.checkbox.border_style = BorderStyle::Solid;
    cfg.checkbox.border_thickness = 1.0f;
    
    cfg.radio = cfg.checkbox;
    
    cfg.slider = cfg.button;
    cfg.slider.background = p.background;
    cfg.slider.foreground = p.accent;
    cfg.slider.foreground_hover = p.accent_hover;
    cfg.slider.foreground_active = p.accent_active;
    cfg.slider.padding_x = 0.0f;
    cfg.slider.padding_y = 0.0f;
    cfg.slider.border_style = BorderStyle::None;
    
    cfg.input.background = p.background;
    cfg.input.background_hover = p.background;
    cfg.input.background_focused = p.background;
    cfg.input.foreground = p.foreground;
    cfg.input.placeholder = p.foreground_dim;
    cfg.input.selection = p.selection;
    cfg.input.cursor = p.accent;
    cfg.input.border = p.border;
    cfg.input.border_hover = p.border_light;
    cfg.input.border_focused = p.border_focus;
    cfg.input.border_thickness = 1.0f;
    cfg.input.rounding = 3.0f;
    cfg.input.padding_x = 8.0f;
    cfg.input.padding_y = 4.0f;
    cfg.input.border_style = BorderStyle::Solid;
    
    cfg.window.background = p.background;
    cfg.window.title_background = p.background_alt;
    cfg.window.title_background_active = p.background_highlight;
    cfg.window.title_foreground = p.foreground;
    cfg.window.border = p.border;
    cfg.window.border_thickness = 1.0f;
    cfg.window.rounding = 4.0f;
    cfg.window.padding = 8.0f;
    cfg.window.title_height = 24.0f;
    cfg.window.border_style = BorderStyle::Solid;
    
    cfg.scrollbar.track = p.background;
    cfg.scrollbar.thumb = p.border_light;
    cfg.scrollbar.thumb_hover = p.foreground_dim;
    cfg.scrollbar.thumb_active = p.accent;
    cfg.scrollbar.width = 10.0f;
    cfg.scrollbar.rounding = 3.0f;
    cfg.scrollbar.min_thumb_size = 20.0f;
    
    cfg.menu.background = p.background_alt;
    cfg.menu.item_hover = p.background_highlight;
    cfg.menu.item_active = p.accent;
    cfg.menu.foreground = p.foreground;
    cfg.menu.foreground_hover = p.foreground;
    cfg.menu.foreground_disabled = p.foreground_disabled;
    cfg.menu.separator = p.border;
    cfg.menu.border = p.border;
    cfg.menu.border_thickness = 1.0f;
    cfg.menu.rounding = 4.0f;
    cfg.menu.item_padding_x = 8.0f;
    cfg.menu.item_padding_y = 4.0f;
    
    cfg.tab.background = p.background;
    cfg.tab.background_hover = p.background_alt;
    cfg.tab.background_active = p.background_alt;
    cfg.tab.foreground = p.foreground_dim;
    cfg.tab.foreground_active = p.foreground;
    cfg.tab.border = p.border;
    cfg.tab.underline = p.accent;
    cfg.tab.rounding = 3.0f;
    cfg.tab.padding_x = 12.0f;
    cfg.tab.padding_y = 6.0f;
    
    cfg.font_size = 14.0f;
    cfg.item_spacing_x = 8.0f;
    cfg.item_spacing_y = 4.0f;
    cfg.indent_spacing = 20.0f;
    cfg.pixel_perfect = false;

    return cfg;
}

void ThemePlugin::register_commands() {
    CommandRegistry* registry = get_context()->get_command_registry();
    if (!registry) {
        return;
    }

    CommandInfo retro_info;
    retro_info.name = "Theme: Retro";
    retro_info.description = "Switch to retro 8-bit theme with classic 90s aesthetics";
    retro_info.shortcut = nullptr;
    retro_info.category = CommandCategory::View;
    
    registry->register_command(retro_info, [](void*) {
        if (ThemePlugin::instance()) {
            ThemePlugin::instance()->set_preset(ThemePreset::Retro);
        }
    }, nullptr);

    CommandInfo minimalist_info;
    minimalist_info.name = "Theme: Dark Minimalist";
    minimalist_info.description = "Switch to dark minimalist theme";
    minimalist_info.shortcut = nullptr;
    minimalist_info.category = CommandCategory::View;
    
    registry->register_command(minimalist_info, [](void*) {
        if (ThemePlugin::instance()) {
            ThemePlugin::instance()->set_preset(ThemePreset::DarkMinimalist);
        }
    }, nullptr);
}

}

extern "C" {

LUNARIS_PLUGIN_EXPORT lunaris::Plugin* lunaris_create_plugin() {
    return new lunaris::ThemePlugin();
}

LUNARIS_PLUGIN_EXPORT void lunaris_destroy_plugin(lunaris::Plugin* plugin) {
    delete plugin;
}

LUNARIS_PLUGIN_EXPORT uint32_t lunaris_get_api_version() {
    return LUNARIS_PLUGIN_API_VERSION;
}

}
