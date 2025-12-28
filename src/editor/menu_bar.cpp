#include "lunaris/editor/menu_bar.h"
#include "lunaris/core/theme.h"
#include "lunaris/core/command_registry.h"
#include "lunaris/plugin/plugin_manager.h"
#include <imgui.h>

namespace lunaris {

MenuBar::MenuBar()
    : _theme(nullptr)
    , _plugin_manager(nullptr)
    , _command_registry(nullptr) {
}

MenuBar::~MenuBar() {
}

void MenuBar::on_init() {
}

void MenuBar::on_shutdown() {
}

void MenuBar::on_ui() {
    if (ImGui::BeginMainMenuBar()) {
        draw_file_menu();
        draw_edit_menu();
        draw_view_menu();
        draw_build_menu();
        draw_debug_menu();
        draw_plugins_menu();
        draw_help_menu();

        ImGui::EndMainMenuBar();
    }
}

void MenuBar::draw_file_menu() {
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New File", "Ctrl+N")) {
            if (_command_registry) {
                _command_registry->execute_command_by_name("New File");
            }
        }
        if (ImGui::MenuItem("Open File...", "Ctrl+O")) {
            if (_command_registry) {
                _command_registry->execute_command_by_name("Open File");
            }
        }
        if (ImGui::MenuItem("Open Folder...")) {
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Save", "Ctrl+S")) {
            if (_command_registry) {
                _command_registry->execute_command_by_name("Save");
            }
        }
        if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
            if (_command_registry) {
                _command_registry->execute_command_by_name("Save As");
            }
        }
        if (ImGui::MenuItem("Save All")) {
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Close File", "Ctrl+W")) {
            if (_command_registry) {
                _command_registry->execute_command_by_name("Close File");
            }
        }
        if (ImGui::MenuItem("Close All")) {
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Quit", "Ctrl+Q")) {
            if (_command_registry) {
                _command_registry->execute_command_by_name("Quit");
            }
        }

        ImGui::EndMenu();
    }
}

void MenuBar::draw_edit_menu() {
    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
        }
        if (ImGui::MenuItem("Redo", "Ctrl+Shift+Z")) {
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Cut", "Ctrl+X")) {
        }
        if (ImGui::MenuItem("Copy", "Ctrl+C")) {
        }
        if (ImGui::MenuItem("Paste", "Ctrl+V")) {
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Find", "Ctrl+F")) {
        }
        if (ImGui::MenuItem("Replace", "Ctrl+H")) {
        }
        if (ImGui::MenuItem("Find in Files", "Ctrl+Shift+F")) {
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Preferences...")) {
        }

        ImGui::EndMenu();
    }
}

void MenuBar::draw_view_menu() {
    if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem("Command Palette", "Ctrl+P")) {
            if (_command_registry) {
                _command_registry->execute_command_by_name("Command Palette");
            }
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Toggle Sidebar", "Ctrl+B")) {
            if (_command_registry) {
                _command_registry->execute_command_by_name("Toggle Sidebar");
            }
        }
        if (ImGui::MenuItem("Toggle Panel", "Ctrl+`")) {
            if (_command_registry) {
                _command_registry->execute_command_by_name("Toggle Panel");
            }
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("Appearance")) {
            if (ImGui::MenuItem("Zoom In", "Ctrl++")) {
            }
            if (ImGui::MenuItem("Zoom Out", "Ctrl+-")) {
            }
            if (ImGui::MenuItem("Reset Zoom", "Ctrl+0")) {
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }
}

void MenuBar::draw_build_menu() {
    if (ImGui::BeginMenu("Build")) {
        if (ImGui::MenuItem("Build Project", "Ctrl+B")) {
        }
        if (ImGui::MenuItem("Rebuild Project")) {
        }
        if (ImGui::MenuItem("Clean")) {
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Run", "Ctrl+R")) {
        }
        if (ImGui::MenuItem("Stop")) {
        }

        ImGui::EndMenu();
    }
}

void MenuBar::draw_debug_menu() {
    if (ImGui::BeginMenu("Debug")) {
        if (ImGui::MenuItem("Start Debugging", "F5")) {
        }
        if (ImGui::MenuItem("Start Without Debugging", "Ctrl+F5")) {
        }
        if (ImGui::MenuItem("Stop Debugging", "Shift+F5")) {
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Toggle Breakpoint", "F9")) {
        }
        if (ImGui::MenuItem("Step Over", "F10")) {
        }
        if (ImGui::MenuItem("Step Into", "F11")) {
        }
        if (ImGui::MenuItem("Step Out", "Shift+F11")) {
        }

        ImGui::EndMenu();
    }
}

void MenuBar::draw_plugins_menu() {
    if (ImGui::BeginMenu("Plugins")) {
        if (ImGui::MenuItem("Manage Plugins...")) {
        }

        if (_plugin_manager && _plugin_manager->get_plugin_count() > 0) {
            ImGui::Separator();
            _plugin_manager->menu_bar_all();
        }

        ImGui::EndMenu();
    }
}

void MenuBar::draw_help_menu() {
    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("Documentation")) {
        }
        if (ImGui::MenuItem("Keyboard Shortcuts")) {
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Check for Updates")) {
        }

        ImGui::Separator();

        if (ImGui::MenuItem("About Lunaris")) {
        }

        ImGui::EndMenu();
    }
}

}
