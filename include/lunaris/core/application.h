#pragma once

#include <tinyvk/tinyvk.h>

namespace lunaris {

class EditorLayer;

class Application : public tvk::App {
public:
    Application();
    ~Application() override;

protected:
    void OnStart() override;
    void OnStop() override;
    void OnUI() override;
    void OnUpdate() override;

private:
    EditorLayer* _editor_layer;
};

}
