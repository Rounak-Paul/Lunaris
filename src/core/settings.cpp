#include "lunaris/core/settings.h"
#include <imgui.h>

namespace lunaris {

Settings* Settings::_instance = nullptr;

Settings* Settings::get() {
    if (!_instance) {
        _instance = new Settings();
    }
    return _instance;
}

Settings::Settings()
    : _ui_scale(DEFAULT_UI_SCALE) {
}

Settings::~Settings() {
}

void Settings::set_ui_scale(float scale) {
    if (scale < MIN_UI_SCALE) scale = MIN_UI_SCALE;
    if (scale > MAX_UI_SCALE) scale = MAX_UI_SCALE;
    _ui_scale = scale;
    apply();
}

void Settings::zoom_in() {
    set_ui_scale(_ui_scale + SCALE_STEP);
}

void Settings::zoom_out() {
    set_ui_scale(_ui_scale - SCALE_STEP);
}

void Settings::reset_zoom() {
    set_ui_scale(DEFAULT_UI_SCALE);
}

void Settings::apply() {
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = _ui_scale;
}

}
