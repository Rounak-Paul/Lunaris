#pragma once

namespace lunaris {

class Settings {
public:
    static constexpr float MIN_UI_SCALE = 0.5f;
    static constexpr float MAX_UI_SCALE = 2.0f;
    static constexpr float DEFAULT_UI_SCALE = 1.0f;
    static constexpr float SCALE_STEP = 0.1f;

    static Settings* get();

    float get_ui_scale() const { return _ui_scale; }
    void set_ui_scale(float scale);

    void zoom_in();
    void zoom_out();
    void reset_zoom();

    void apply();

private:
    Settings();
    ~Settings();

    float _ui_scale;

    static Settings* _instance;
};

}
