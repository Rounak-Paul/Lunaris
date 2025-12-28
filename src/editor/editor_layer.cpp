#include "lunaris/editor/editor_layer.h"
#include "lunaris/editor/workspace.h"
#include "lunaris/plugin/plugin_manager.h"
#include "lunaris/plugin/editor_context.h"
#include "lunaris/core/job_system.h"
#include <imgui.h>
#include <imgui_internal.h>

namespace lunaris {

EditorLayer::EditorLayer()
    : _workspace(nullptr)
    , _plugin_manager(nullptr)
    , _context(nullptr)
    , _job_system(nullptr)
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

    _job_system->init();

    _context->set_workspace(_workspace);
    _context->set_plugin_manager(_plugin_manager);
    _context->set_job_system(_job_system);
    _plugin_manager->set_context(_context);
    _workspace->set_plugin_manager(_plugin_manager);

    _workspace->on_init();
}

void EditorLayer::on_shutdown() {
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
    draw_menu_bar();
    draw_workspace();

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

    if (_first_frame) {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);
        ImGui::DockBuilderSetNodePos(dockspace_id, viewport->Pos);

        ImGui::DockBuilderDockWindow("Workspace", dockspace_id);
        ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
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

void EditorLayer::draw_menu_bar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
            }
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "Ctrl+X")) {
            }
            if (ImGui::MenuItem("Copy", "Ctrl+C")) {
            }
            if (ImGui::MenuItem("Paste", "Ctrl+V")) {
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Demo Window", nullptr, &_show_demo_window);
            ImGui::EndMenu();
        }

        if (_plugin_manager) {
            _plugin_manager->menu_bar_all();
        }

        ImGui::EndMainMenuBar();
    }
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

    bool open = true;
    if (ImGui::Begin("Workspace", &open, flags)) {
        if (_workspace) {
            _workspace->on_ui();
        }
    }
    ImGui::End();
}

}
