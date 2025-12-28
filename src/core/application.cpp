#include "lunaris/core/application.h"
#include "lunaris/editor/editor_layer.h"

namespace lunaris {

Application::Application()
    : _editor_layer(nullptr) {
}

Application::~Application() {
}

void Application::OnStart() {
    _editor_layer = new EditorLayer();
    _editor_layer->on_init();
}

void Application::OnStop() {
    if (_editor_layer) {
        _editor_layer->on_shutdown();
        delete _editor_layer;
        _editor_layer = nullptr;
    }
}

void Application::OnUI() {
    if (_editor_layer) {
        _editor_layer->on_ui();
    }
}

void Application::OnUpdate() {
    if (_editor_layer) {
        _editor_layer->on_update(DeltaTime());
    }
}

}
