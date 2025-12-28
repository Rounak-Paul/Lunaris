#include "lunaris/editor/editor_layer.h"
#include "lunaris/editor/workspace.h"
#include "lunaris/editor/status_bar.h"
#include "lunaris/editor/activity_bar.h"
#include "lunaris/plugin/plugin_manager.h"
#include "lunaris/plugin/editor_context.h"
#include "lunaris/core/job_system.h"
#include "lunaris/core/theme.h"
#include "lunaris/ui/components.h"
#include <imgui.h>
#include <imgui_internal.h>

namespace lunaris {

EditorLayer::EditorLayer()
    : _workspace(nullptr)
    , _status_bar(nullptr)
    , _activity_bar(nullptr)
    , _plugin_manager(nullptr)
    , _context(nullptr)
    , _job_system(nullptr)
    , _theme(nullptr)
    , _show_demo_window(false)
    , _first_frame(true) {
}

EditorLayer::~EditorLayer() {
}

void EditorLayer::on_init() {
    _context = new EditorContext();
    _job_system = new JobSystem();
    _plugin_manager = new PluginManager();
    _workspace = new Workspace();
    _status_bar = new StatusBar();
    _activity_bar = new ActivityBar();
    _theme = new Theme();

    _job_system->init();
    _theme->apply();
    ui::init(_theme);

    _context->set_workspace(_workspace);
    _context->set_plugin_manager(_plugin_manager);
    _context->set_job_system(_job_system);
    _context->set_theme(_theme);
    _plugin_manager->set_context(_context);
    _workspace->set_plugin_manager(_plugin_manager);
    _status_bar->set_plugin_manager(_plugin_manager);
    _activity_bar->set_plugin_manager(_plugin_manager);
    _activity_bar->set_theme(_theme);

    _workspace->on_init();
    _status_bar->on_init();
    _activity_bar->on_init();
}

void EditorLayer::on_shutdown() {
    ui::shutdown();

    if (_activity_bar) {
        _activity_bar->on_shutdown();
        delete _activity_bar;
        _activity_bar = nullptr;
    }

    if (_status_bar) {
        _status_bar->on_shutdown();
        delete _status_bar;
        _status_bar = nullptr;
    }

    if (_workspace) {
        _workspace->on_shutdown();
        delete _workspace;
        _workspace = nullptr;
    }

    if (_plugin_manager) {
        _plugin_manager->unregister_all();
        delete _plugin_manager;
        _plugin_manager = nullptr;
    }

    if (_job_system) {
        _job_system->shutdown();
        delete _job_system;
        _job_system = nullptr;
    }

    if (_theme) {
        delete _theme;
        _theme = nullptr;
    }

    if (_context) {
        delete _context;
        _context = nullptr;
    }
}

void EditorLayer::on_update(float delta_time) {
    if (_plugin_manager) {
        _plugin_manager->update_all(delta_time);
    }

    if (_workspace) {
        _workspace->on_update(delta_time);
    }
}

void EditorLayer::on_ui() {
    setup_dockspace();
    draw_activity_bar();
    draw_workspace();
    draw_status_bar();

    if (_plugin_manager) {
        _plugin_manager->ui_all();
    }

    if (_show_demo_window) {
        ImGui::ShowDemoWindow(&_show_demo_window);
    }

    _first_frame = false;
}

void EditorLayer::setup_dockspace() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGuiID dockspace_id = ImGui::GetID("LunarisDockspace");

    constexpr float status_bar_height = 26.0f;
    constexpr float activity_bar_width = 48.0f;

    if (_first_frame) {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(viewport->WorkSize.x - activity_bar_width, viewport->WorkSize.y - status_bar_height));
        ImGui::DockBuilderSetNodePos(dockspace_id, ImVec2(viewport->WorkPos.x + activity_bar_width, viewport->WorkPos.y));

        ImGui::DockBuilderDockWindow("Workspace", dockspace_id);
        ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + activity_bar_width, viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - activity_bar_width, viewport->WorkSize.y - status_bar_height));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags host_flags = ImGuiWindowFlags_NoTitleBar
                                | ImGuiWindowFlags_NoCollapse
                                | ImGuiWindowFlags_NoResize
                                | ImGuiWindowFlags_NoMove
                                | ImGuiWindowFlags_NoDocking
                                | ImGuiWindowFlags_NoBringToFrontOnFocus
                                | ImGuiWindowFlags_NoNavFocus
                                | ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockspaceHost", nullptr, host_flags);
    ImGui::PopStyleVar(3);

    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    ImGui::End();
}

void EditorLayer::draw_activity_bar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    constexpr float activity_bar_width = 48.0f;
    constexpr float status_bar_height = 26.0f;

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(activity_bar_width, viewport->WorkSize.y - status_bar_height));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar
                           | ImGuiWindowFlags_NoCollapse
                           | ImGuiWindowFlags_NoResize
                           | ImGuiWindowFlags_NoMove
                           | ImGuiWindowFlags_NoDocking
                           | ImGuiWindowFlags_NoScrollbar
                           | ImGuiWindowFlags_NoScrollWithMouse
                           | ImGuiWindowFlags_NoBringToFrontOnFocus
                           | ImGuiWindowFlags_NoNavFocus;

    Color bar_bg = _theme ? _theme->get_background() : Color(0.1f, 0.1f, 0.12f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(bar_bg.r, bar_bg.g, bar_bg.b, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 8.0f));

    if (ImGui::Begin("##ActivityBar", nullptr, flags)) {
        if (_activity_bar) {
            _activity_bar->on_ui();
        }
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();
}

void EditorLayer::draw_workspace() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse
                           | ImGuiWindowFlags_NoMove
                           | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar
                                          | ImGuiDockNodeFlags_NoDockingOverMe
                                          | ImGuiDockNodeFlags_NoDockingSplit;

    ImGui::SetNextWindowClass(&window_class);

    Color workspace_bg = _theme ? _theme->get_background().darken(0.03f) : Color(0.05f, 0.05f, 0.06f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(workspace_bg.r, workspace_bg.g, workspace_bg.b, 1.0f));

    bool open = true;
    if (ImGui::Begin("Workspace", &open, flags)) {
        if (_workspace) {
            _workspace->on_ui();
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();
}

void EditorLayer::draw_status_bar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    constexpr float status_bar_height = 26.0f;

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - status_bar_height));
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, status_bar_height));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar
                           | ImGuiWindowFlags_NoCollapse
                           | ImGuiWindowFlags_NoResize
                           | ImGuiWindowFlags_NoMove
                           | ImGuiWindowFlags_NoDocking
                           | ImGuiWindowFlags_NoScrollbar
                           | ImGuiWindowFlags_NoScrollWithMouse
                           | ImGuiWindowFlags_NoBringToFrontOnFocus
                           | ImGuiWindowFlags_NoNavFocus;

    Color status_bg = _theme ? _theme->get_surface() : Color(0.14f, 0.14f, 0.16f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(status_bg.r, status_bg.g, status_bg.b, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 4.0f));

    if (ImGui::Begin("##StatusBar", nullptr, flags)) {
        if (_status_bar) {
            _status_bar->on_ui();
        }
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();
}

}
