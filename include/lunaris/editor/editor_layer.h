#pragma once

namespace lunaris {

class EditorLayer {
public:
    EditorLayer();
    ~EditorLayer();

    void on_init();
    void on_shutdown();
    void on_update(float delta_time);
    void on_ui();

private:
    bool _show_demo_window;
};

}
