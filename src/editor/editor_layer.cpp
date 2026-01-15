#include "lunaris/editor/editor_layer.h"
#include "lunaris/editor/workspace.h"
#include "lunaris/editor/status_bar.h"
#include "lunaris/editor/menu_bar.h"
#include "lunaris/editor/sidebar.h"
#include "lunaris/editor/tab_bar.h"
#include "lunaris/editor/bottom_panel.h"
#include "lunaris/editor/command_palette.h"
#include "lunaris/editor/document_manager.h"
#include "lunaris/editor/document.h"
#include "lunaris/editor/text_editor.h"
#include "lunaris/editor/file_operations.h"
#include "lunaris/plugin/plugin_manager.h"
#include "lunaris/plugin/editor_context.h"
#include "lunaris/core/job_system.h"
#include "lunaris/core/theme.h"
#include "lunaris/core/command_registry.h"
#include "lunaris/core/settings.h"
#include "lunaris/ui/components.h"
#include <imgui.h>
#include <imgui_internal.h>

namespace lunaris {

static EditorLayer* s_instance = nullptr;

EditorLayer::EditorLayer()
    : _workspace(nullptr)
    , _status_bar(nullptr)
    , _menu_bar(nullptr)
    , _sidebar(nullptr)
    , _tab_bar(nullptr)
    , _bottom_panel(nullptr)
    , _command_registry(nullptr)
    , _command_palette(nullptr)
    , _plugin_manager(nullptr)
    , _context(nullptr)
    , _job_system(nullptr)
    , _theme(nullptr)
    , _document_manager(nullptr)
    , _text_editor(nullptr)
    , _file_operations(nullptr)
    , _first_frame(true) {
    s_instance = this;
}

EditorLayer::~EditorLayer() {
    s_instance = nullptr;
}

void EditorLayer::on_init() {
    _context = new EditorContext();
    _job_system = new JobSystem();
    _plugin_manager = new PluginManager();
    _command_registry = new CommandRegistry();
    _command_palette = new CommandPalette();
    _menu_bar = new MenuBar();
    _sidebar = new Sidebar();
    _tab_bar = new TabBar();
    _bottom_panel = new BottomPanel();
    _workspace = new Workspace();
    _status_bar = new StatusBar();
    _theme = new Theme();
    _document_manager = new DocumentManager();
    _text_editor = new TextEditor();
    _file_operations = new FileOperations();

    _job_system->init();
    _theme->apply();
    ui::init(_theme);

    _context->set_workspace(_workspace);
    _context->set_plugin_manager(_plugin_manager);
    _context->set_job_system(_job_system);
    _context->set_theme(_theme);
    _context->set_command_registry(_command_registry);

    _plugin_manager->set_context(_context);
    _menu_bar->set_theme(_theme);
    _menu_bar->set_plugin_manager(_plugin_manager);
    _menu_bar->set_command_registry(_command_registry);
    _sidebar->set_theme(_theme);
    _sidebar->set_plugin_manager(_plugin_manager);
    _sidebar->set_file_operations(_file_operations);
    _sidebar->set_file_selected_callback([](const char* path, void* user_data) {
        EditorLayer* editor = static_cast<EditorLayer*>(user_data);
        if (editor) {
            editor->open_file(path);
        }
    }, this);
    _tab_bar->set_theme(_theme);
    _tab_bar->set_on_tab_selected([](TabID id, void* user_data) {
        EditorLayer* editor = static_cast<EditorLayer*>(user_data);
        if (editor && editor->_document_manager) {
            editor->_document_manager->on_tab_selected(id);
        }
    }, this);
    _tab_bar->set_on_tab_closed([](TabID id, void* user_data) {
        EditorLayer* editor = static_cast<EditorLayer*>(user_data);
        if (editor && editor->_document_manager) {
            editor->_document_manager->on_tab_closed(id);
        }
    }, this);
    _bottom_panel->set_theme(_theme);
    _workspace->set_plugin_manager(_plugin_manager);
    _status_bar->set_plugin_manager(_plugin_manager);
    _command_palette->set_command_registry(_command_registry);
    _command_palette->set_theme(_theme);
    _document_manager->set_tab_bar(_tab_bar);
    _document_manager->set_theme(_theme);
    _text_editor->set_theme(_theme);
    _text_editor->set_document_manager(_document_manager);
    _text_editor->set_file_operations(_file_operations);
    _file_operations->set_document_manager(_document_manager);
    _file_operations->set_sidebar(_sidebar);

    register_builtin_commands();

    _plugin_manager->load_plugins_from_directory("plugins");

    _menu_bar->on_init();
    _sidebar->on_init();
    _tab_bar->on_init();
    _bottom_panel->on_init();
    _workspace->on_init();
    _status_bar->on_init();
}

void EditorLayer::on_shutdown() {
    ui::shutdown();

    if (_file_operations) {
        delete _file_operations;
        _file_operations = nullptr;
    }

    if (_text_editor) {
        delete _text_editor;
        _text_editor = nullptr;
    }

    if (_document_manager) {
        delete _document_manager;
        _document_manager = nullptr;
    }

    if (_command_palette) {
        delete _command_palette;
        _command_palette = nullptr;
    }

    if (_command_registry) {
        delete _command_registry;
        _command_registry = nullptr;
    }

    if (_menu_bar) {
        _menu_bar->on_shutdown();
        delete _menu_bar;
        _menu_bar = nullptr;
    }

    if (_sidebar) {
        _sidebar->on_shutdown();
        delete _sidebar;
        _sidebar = nullptr;
    }

    if (_tab_bar) {
        _tab_bar->on_shutdown();
        delete _tab_bar;
        _tab_bar = nullptr;
    }

    if (_bottom_panel) {
        _bottom_panel->on_shutdown();
        delete _bottom_panel;
        _bottom_panel = nullptr;
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
    handle_keyboard_shortcuts();

    if (_menu_bar) {
        _menu_bar->on_ui();
    }

    setup_layout();

    if (_command_palette) {
        _command_palette->on_ui();
    }

    if (_plugin_manager) {
        _plugin_manager->ui_all();
    }

    _first_frame = false;
}

void EditorLayer::handle_keyboard_shortcuts() {
    ImGuiIO& io = ImGui::GetIO();
    bool ctrl = io.KeyCtrl || io.KeySuper;
    bool shift = io.KeyShift;

    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_P, false)) {
        if (_command_palette && !_command_palette->is_open()) {
            _command_palette->toggle();
        }
    }

    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_N, false)) {
        new_file();
    }

    if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_O, false)) {
        if (_document_manager) {
            _document_manager->open_file_dialog();
        }
    }

    if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
        save_file();
    }

    if (ctrl && shift && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
        save_file_as();
    }

    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_W, false)) {
        close_file();
    }

    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_B, false)) {
        if (_sidebar) {
            _sidebar->toggle();
        }
    }

    if (ctrl && shift && ImGui::IsKeyPressed(ImGuiKey_O, false)) {
        if (_sidebar) {
            _sidebar->open_folder();
        }
    }

    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_GraveAccent, false)) {
        if (_bottom_panel) {
            _bottom_panel->toggle();
        }
    }

    if (ctrl && shift && ImGui::IsKeyPressed(ImGuiKey_Equal, false)) {
        Settings::get()->zoom_in();
    }

    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Minus, false)) {
        Settings::get()->zoom_out();
    }

    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_0, false)) {
        Settings::get()->reset_zoom();
    }
}

void EditorLayer::register_builtin_commands() {
    CommandInfo cmd_palette;
    cmd_palette.name = "Command Palette";
    cmd_palette.description = "Open the command palette";
    cmd_palette.shortcut = "Ctrl+P";
    cmd_palette.category = CommandCategory::General;
    _command_registry->register_command(cmd_palette, [](void*) {
        if (s_instance && s_instance->_command_palette) {
            s_instance->_command_palette->toggle();
        }
    }, nullptr);

    CommandInfo cmd_sidebar;
    cmd_sidebar.name = "Toggle Sidebar";
    cmd_sidebar.description = "Show or hide the sidebar panel";
    cmd_sidebar.shortcut = "Ctrl+B";
    cmd_sidebar.category = CommandCategory::View;
    _command_registry->register_command(cmd_sidebar, [](void*) {
        if (s_instance && s_instance->_sidebar) {
            s_instance->_sidebar->toggle();
        }
    }, nullptr);

    CommandInfo cmd_panel;
    cmd_panel.name = "Toggle Panel";
    cmd_panel.description = "Show or hide the bottom panel";
    cmd_panel.shortcut = "Ctrl+`";
    cmd_panel.category = CommandCategory::View;
    _command_registry->register_command(cmd_panel, [](void*) {
        if (s_instance && s_instance->_bottom_panel) {
            s_instance->_bottom_panel->toggle();
        }
    }, nullptr);

    CommandInfo cmd_new;
    cmd_new.name = "New File";
    cmd_new.description = "Create a new file";
    cmd_new.shortcut = "Ctrl+N";
    cmd_new.category = CommandCategory::File;
    _command_registry->register_command(cmd_new, [](void*) {
        if (s_instance) {
            s_instance->new_file();
        }
    }, nullptr);

    CommandInfo cmd_open;
    cmd_open.name = "Open File";
    cmd_open.description = "Open an existing file";
    cmd_open.shortcut = "Ctrl+O";
    cmd_open.category = CommandCategory::File;
    _command_registry->register_command(cmd_open, [](void*) {
        if (s_instance && s_instance->_document_manager) {
            s_instance->_document_manager->open_file_dialog();
        }
    }, nullptr);

    CommandInfo cmd_open_folder;
    cmd_open_folder.name = "Open Folder";
    cmd_open_folder.description = "Open a folder in the explorer";
    cmd_open_folder.shortcut = "Ctrl+Shift+O";
    cmd_open_folder.category = CommandCategory::File;
    _command_registry->register_command(cmd_open_folder, [](void*) {
        if (s_instance && s_instance->_sidebar) {
            s_instance->_sidebar->open_folder();
        }
    }, nullptr);

    CommandInfo cmd_save;
    cmd_save.name = "Save";
    cmd_save.description = "Save the current file";
    cmd_save.shortcut = "Ctrl+S";
    cmd_save.category = CommandCategory::File;
    _command_registry->register_command(cmd_save, [](void*) {
        if (s_instance) {
            s_instance->save_file();
        }
    }, nullptr);

    CommandInfo cmd_save_as;
    cmd_save_as.name = "Save As";
    cmd_save_as.description = "Save the current file with a new name";
    cmd_save_as.shortcut = "Ctrl+Shift+S";
    cmd_save_as.category = CommandCategory::File;
    _command_registry->register_command(cmd_save_as, [](void*) {
        if (s_instance) {
            s_instance->save_file_as();
        }
    }, nullptr);

    CommandInfo cmd_close;
    cmd_close.name = "Close File";
    cmd_close.description = "Close the current file";
    cmd_close.shortcut = "Ctrl+W";
    cmd_close.category = CommandCategory::File;
    _command_registry->register_command(cmd_close, [](void*) {
        if (s_instance) {
            s_instance->close_file();
        }
    }, nullptr);

    CommandInfo cmd_quit;
    cmd_quit.name = "Quit";
    cmd_quit.description = "Exit the application";
    cmd_quit.shortcut = "Ctrl+Q";
    cmd_quit.category = CommandCategory::General;
    _command_registry->register_command(cmd_quit, [](void*) {}, nullptr);

    CommandInfo cmd_find;
    cmd_find.name = "Find";
    cmd_find.description = "Find in current file";
    cmd_find.shortcut = "Ctrl+F";
    cmd_find.category = CommandCategory::Search;
    _command_registry->register_command(cmd_find, [](void*) {}, nullptr);

    CommandInfo cmd_goto_line;
    cmd_goto_line.name = "Go to Line";
    cmd_goto_line.description = "Jump to a specific line number";
    cmd_goto_line.shortcut = "Ctrl+G";
    cmd_goto_line.category = CommandCategory::Navigation;
    _command_registry->register_command(cmd_goto_line, [](void*) {}, nullptr);

    CommandInfo cmd_zoom_in;
    cmd_zoom_in.name = "Zoom In";
    cmd_zoom_in.description = "Increase the UI scale";
    cmd_zoom_in.shortcut = "Ctrl+Shift+=";
    cmd_zoom_in.category = CommandCategory::View;
    _command_registry->register_command(cmd_zoom_in, [](void*) {
        Settings::get()->zoom_in();
    }, nullptr);

    CommandInfo cmd_zoom_out;
    cmd_zoom_out.name = "Zoom Out";
    cmd_zoom_out.description = "Decrease the UI scale";
    cmd_zoom_out.shortcut = "Ctrl+-";
    cmd_zoom_out.category = CommandCategory::View;
    _command_registry->register_command(cmd_zoom_out, [](void*) {
        Settings::get()->zoom_out();
    }, nullptr);

    CommandInfo cmd_reset_zoom;
    cmd_reset_zoom.name = "Reset Zoom";
    cmd_reset_zoom.description = "Reset the UI scale to default";
    cmd_reset_zoom.shortcut = "Ctrl+0";
    cmd_reset_zoom.category = CommandCategory::View;
    _command_registry->register_command(cmd_reset_zoom, [](void*) {
        Settings::get()->reset_zoom();
    }, nullptr);
}

void EditorLayer::setup_layout() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    float font_size = ImGui::GetFontSize();
    float status_height = font_size * 1.5f;
    float sidebar_width = _sidebar ? _sidebar->get_width() : 0.0f;
    float panel_height = (_bottom_panel && _bottom_panel->is_visible()) ? _bottom_panel->get_height() : 0.0f;

    Color bg = _theme ? _theme->get_background() : Color(0.1f, 0.1f, 0.12f);
    Color surface = _theme ? _theme->get_surface() : Color(0.12f, 0.12f, 0.14f);

    ImGuiWindowFlags layout_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - status_height));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(bg.r, bg.g, bg.b, 1.0f));

    if (ImGui::Begin("##MainLayout", nullptr, layout_flags)) {
        if (_sidebar) {
            ImGui::BeginChild("##Sidebar", ImVec2(sidebar_width, 0.0f), false, ImGuiWindowFlags_NoScrollbar);
            _sidebar->on_ui();
            ImGui::EndChild();
            ImGui::SameLine(0.0f, 0.0f);
        }

        float editor_w = ImGui::GetContentRegionAvail().x;
        ImGui::BeginChild("##EditorArea", ImVec2(editor_w, 0.0f), false, ImGuiWindowFlags_NoScrollbar);
        {
            if (_tab_bar && _tab_bar->has_tabs()) {
                _tab_bar->on_ui();
            }

            float workspace_h = ImGui::GetContentRegionAvail().y - panel_height;
            ImGui::BeginChild("##Workspace", ImVec2(0.0f, workspace_h), false, ImGuiWindowFlags_NoScrollbar);
            
            if (_document_manager && _document_manager->get_document_count() > 0) {
                Document* active_doc = _document_manager->get_active_document();
                if (active_doc && _text_editor) {
                    _text_editor->set_document(active_doc);
                    _text_editor->on_ui();
                }
            } else if (_workspace) {
                _workspace->on_ui();
            }
            
            ImGui::EndChild();

            if (_bottom_panel && _bottom_panel->is_visible()) {
                _bottom_panel->on_ui();
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopStyleColor();

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - status_height));
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, status_height));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(surface.r, surface.g, surface.b, 1.0f));

    if (ImGui::Begin("##StatusBar", nullptr, layout_flags)) {
        float y = (status_height - ImGui::GetTextLineHeight()) * 0.5f;
        ImGui::SetCursorPos(ImVec2(font_size * 0.75f, y));
        if (_status_bar) {
            _status_bar->on_ui();
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();

    ImGui::PopStyleVar(4);
}

void EditorLayer::open_file(const char* filepath) {
    if (_document_manager) {
        _document_manager->open_document(filepath);
    }
}

void EditorLayer::new_file() {
    if (_document_manager) {
        _document_manager->new_document();
    }
}

void EditorLayer::save_file() {
    if (!_document_manager) {
        return;
    }
    
    Document* doc = _document_manager->get_active_document();
    if (!doc) {
        return;
    }
    
    if (doc->has_file()) {
        _document_manager->save_active_document();
    } else {
        save_file_as();
    }
}

void EditorLayer::save_file_as() {
    if (_document_manager) {
        _document_manager->save_file_dialog();
    }
}

void EditorLayer::close_file() {
    if (_document_manager) {
        _document_manager->close_active_document();
    }
}

}
