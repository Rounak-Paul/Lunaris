#include "theme_plugin.h"
#include "lunaris/plugin/plugin_api.h"
#include "lunaris/plugin/editor_context.h"
#include "lunaris/core/command_registry.h"
#include "lunaris/core/theme.h"
#include <imgui.h>

namespace lunaris {

ThemePlugin* ThemePlugin::s_instance = nullptr;

ThemePlugin::ThemePlugin()
    : _current_preset(ThemePreset::Default) {
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
    info.description = "Built-in theme switcher with multiple presets";
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

    switch (preset) {
        case ThemePreset::Minimalist:
            apply_minimalist(theme);
            break;
        case ThemePreset::Retro:
            apply_retro(theme);
            break;
        default:
            apply_default(theme);
            break;
    }
    theme->apply();
}

const char* ThemePlugin::get_preset_name(ThemePreset preset) {
    switch (preset) {
        case ThemePreset::Default: return "Default";
        case ThemePreset::Minimalist: return "Minimalist";
        case ThemePreset::Retro: return "Retro";
        default: return "Unknown";
    }
}

void ThemePlugin::apply_default(Theme* theme) {
    theme->set_style(ThemeStyle::Dark);
    theme->set_primary_color_hex(0x4A9EFF);
}

void ThemePlugin::apply_minimalist(Theme* theme) {
    theme->set_style(ThemeStyle::Dark);
    theme->set_primary_color(Color(0.6f, 0.6f, 0.6f, 1.0f));
    theme->set_background(Color(0.05f, 0.05f, 0.05f, 1.0f));
    theme->set_background_alt(Color(0.07f, 0.07f, 0.07f, 1.0f));
    theme->set_surface(Color(0.1f, 0.1f, 0.1f, 1.0f));
    theme->set_text(Color(0.85f, 0.85f, 0.85f, 1.0f));
    theme->set_text_dim(Color(0.45f, 0.45f, 0.45f, 1.0f));
    theme->set_border(Color(0.15f, 0.15f, 0.15f, 1.0f));
    theme->set_accent(Color(0.6f, 0.6f, 0.6f, 1.0f));
    theme->set_accent_hover(Color(0.75f, 0.75f, 0.75f, 1.0f));
    theme->set_accent_active(Color(0.5f, 0.5f, 0.5f, 1.0f));
    theme->set_success(Color(0.5f, 0.7f, 0.5f, 1.0f));
    theme->set_warning(Color(0.8f, 0.7f, 0.4f, 1.0f));
    theme->set_error(Color(0.8f, 0.4f, 0.4f, 1.0f));
}

void ThemePlugin::apply_retro(Theme* theme) {
    theme->set_style(ThemeStyle::Dark);
    theme->set_primary_color(Color(0.25f, 0.45f, 0.65f, 1.0f));
    theme->set_background(Color(0.75f, 0.75f, 0.75f, 1.0f));
    theme->set_background_alt(Color(0.70f, 0.70f, 0.70f, 1.0f));
    theme->set_surface(Color(0.75f, 0.75f, 0.75f, 1.0f));
    theme->set_text(Color(0.0f, 0.0f, 0.0f, 1.0f));
    theme->set_text_dim(Color(0.35f, 0.35f, 0.35f, 1.0f));
    theme->set_border(Color(0.5f, 0.5f, 0.5f, 1.0f));
    theme->set_accent(Color(0.0f, 0.0f, 0.5f, 1.0f));
    theme->set_accent_hover(Color(0.0f, 0.0f, 0.6f, 1.0f));
    theme->set_accent_active(Color(0.0f, 0.0f, 0.4f, 1.0f));
    theme->set_success(Color(0.0f, 0.5f, 0.0f, 1.0f));
    theme->set_warning(Color(0.6f, 0.5f, 0.0f, 1.0f));
    theme->set_error(Color(0.6f, 0.0f, 0.0f, 1.0f));

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.ChildRounding = 0.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.TabBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;

    style.FramePadding = ImVec2(4.0f, 3.0f);
    style.ItemSpacing = ImVec2(6.0f, 4.0f);
    style.WindowPadding = ImVec2(6.0f, 6.0f);

    ImVec4* colors = style.Colors;

    ImVec4 bg_color = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
    ImVec4 highlight = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 shadow = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
    ImVec4 dark_shadow = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    ImVec4 sunken_bg = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 title_active = ImVec4(0.0f, 0.0f, 0.5f, 1.0f);
    ImVec4 title_inactive = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    ImVec4 text_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    ImVec4 text_disabled = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    ImVec4 selection = ImVec4(0.0f, 0.0f, 0.5f, 1.0f);

    colors[ImGuiCol_Text] = text_color;
    colors[ImGuiCol_TextDisabled] = text_disabled;
    colors[ImGuiCol_WindowBg] = bg_color;
    colors[ImGuiCol_ChildBg] = bg_color;
    colors[ImGuiCol_PopupBg] = bg_color;
    colors[ImGuiCol_Border] = shadow;
    colors[ImGuiCol_BorderShadow] = highlight;

    colors[ImGuiCol_FrameBg] = sunken_bg;
    colors[ImGuiCol_FrameBgHovered] = sunken_bg;
    colors[ImGuiCol_FrameBgActive] = sunken_bg;

    colors[ImGuiCol_TitleBg] = title_inactive;
    colors[ImGuiCol_TitleBgActive] = title_active;
    colors[ImGuiCol_TitleBgCollapsed] = title_inactive;

    colors[ImGuiCol_MenuBarBg] = bg_color;
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.65f, 0.65f, 0.65f, 1.0f);
    colors[ImGuiCol_ScrollbarGrab] = bg_color;
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.78f, 0.78f, 0.78f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.70f, 0.70f, 0.70f, 1.0f);

    colors[ImGuiCol_CheckMark] = text_color;
    colors[ImGuiCol_SliderGrab] = bg_color;
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.70f, 0.70f, 0.70f, 1.0f);

    colors[ImGuiCol_Button] = bg_color;
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.80f, 0.80f, 0.80f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.68f, 0.68f, 0.68f, 1.0f);

    colors[ImGuiCol_Header] = selection;
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.0f, 0.6f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.0f, 0.5f, 1.0f);

    colors[ImGuiCol_Separator] = shadow;
    colors[ImGuiCol_SeparatorHovered] = shadow;
    colors[ImGuiCol_SeparatorActive] = dark_shadow;

    colors[ImGuiCol_ResizeGrip] = bg_color;
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.78f, 0.78f, 0.78f, 1.0f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.70f, 0.70f, 0.70f, 1.0f);

    colors[ImGuiCol_Tab] = bg_color;
    colors[ImGuiCol_TabHovered] = ImVec4(0.80f, 0.80f, 0.80f, 1.0f);
    colors[ImGuiCol_TabSelected] = ImVec4(0.80f, 0.80f, 0.80f, 1.0f);
    colors[ImGuiCol_TabDimmed] = ImVec4(0.68f, 0.68f, 0.68f, 1.0f);
    colors[ImGuiCol_TabDimmedSelected] = bg_color;

    colors[ImGuiCol_DockingPreview] = ImVec4(0.0f, 0.0f, 0.5f, 0.7f);
    colors[ImGuiCol_DockingEmptyBg] = bg_color;

    colors[ImGuiCol_TextSelectedBg] = selection;
    colors[ImGuiCol_NavHighlight] = selection;
}

void ThemePlugin::register_commands() {
    CommandRegistry* registry = get_context()->get_command_registry();
    if (!registry) {
        return;
    }

    CommandInfo cmd_default;
    cmd_default.name = "Theme: Default";
    cmd_default.description = "Switch to the default dark theme";
    cmd_default.shortcut = "";
    cmd_default.category = CommandCategory::View;
    registry->register_command(cmd_default, [](void*) {
        if (s_instance) {
            s_instance->set_preset(ThemePreset::Default);
        }
    }, nullptr);

    CommandInfo cmd_minimalist;
    cmd_minimalist.name = "Theme: Minimalist";
    cmd_minimalist.description = "Switch to a minimal monochrome theme";
    cmd_minimalist.shortcut = "";
    cmd_minimalist.category = CommandCategory::View;
    registry->register_command(cmd_minimalist, [](void*) {
        if (s_instance) {
            s_instance->set_preset(ThemePreset::Minimalist);
        }
    }, nullptr);

    CommandInfo cmd_retro;
    cmd_retro.name = "Theme: Retro";
    cmd_retro.description = "Switch to a retro 1990s style theme with raised buttons";
    cmd_retro.shortcut = "";
    cmd_retro.category = CommandCategory::View;
    registry->register_command(cmd_retro, [](void*) {
        if (s_instance) {
            s_instance->set_preset(ThemePreset::Retro);
        }
    }, nullptr);
}

}

LUNARIS_DECLARE_PLUGIN(lunaris::ThemePlugin)
