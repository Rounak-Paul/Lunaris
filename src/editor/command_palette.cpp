#include "lunaris/editor/command_palette.h"
#include "lunaris/core/command_registry.h"
#include "lunaris/core/theme.h"
#include "lunaris/ui/components.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <cstring>

namespace lunaris {

CommandPalette::CommandPalette()
    : _registry(nullptr)
    , _theme(nullptr)
    , _is_open(false)
    , _focus_input(false)
    , _result_count(0)
    , _selected_index(0) {
    memset(_search_buffer, 0, sizeof(_search_buffer));
    memset(_results, 0, sizeof(_results));
}

CommandPalette::~CommandPalette() {
}

void CommandPalette::open() {
    _is_open = true;
    _focus_input = true;
    _selected_index = 0;
    memset(_search_buffer, 0, sizeof(_search_buffer));
    update_search();
}

void CommandPalette::close() {
    _is_open = false;
    _focus_input = false;
}

void CommandPalette::toggle() {
    if (_is_open) {
        close();
    } else {
        open();
    }
}

void CommandPalette::update_search() {
    if (!_registry) {
        _result_count = 0;
        return;
    }

    _result_count = _registry->search_commands(_search_buffer, _results, MAX_RESULTS);
    if (_selected_index >= _result_count && _result_count > 0) {
        _selected_index = _result_count - 1;
    }
}

void CommandPalette::execute_selected() {
    if (_result_count == 0 || !_registry) {
        return;
    }

    CommandID id = _results[_selected_index];
    close();
    _registry->execute_command(id);
}

void CommandPalette::on_ui() {
    if (!_is_open) {
        return;
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    float palette_x = viewport->WorkPos.x + (viewport->WorkSize.x - WIDTH) * 0.5f;
    float palette_y = viewport->WorkPos.y + 80.0f;

    Color bg = _theme ? _theme->get_surface() : Color(0.12f, 0.12f, 0.14f);
    Color border = _theme ? _theme->get_border() : Color(0.25f, 0.25f, 0.28f);
    Color text = _theme ? _theme->get_text() : Color(0.9f, 0.9f, 0.92f);
    Color accent = _theme ? _theme->get_accent() : Color(0.3f, 0.5f, 0.8f);

    ImGui::SetNextWindowPos(ImVec2(palette_x, palette_y));
    ImGui::SetNextWindowSize(ImVec2(WIDTH, 0.0f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar
                           | ImGuiWindowFlags_NoResize
                           | ImGuiWindowFlags_NoMove
                           | ImGuiWindowFlags_NoScrollbar
                           | ImGuiWindowFlags_NoSavedSettings
                           | ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(bg.r, bg.g, bg.b, 0.98f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(border.r, border.g, border.b, 0.8f));

    if (ImGui::Begin("##CommandPalette", nullptr, flags)) {
        draw_input();
        draw_results();

        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            close();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && _result_count > 0) {
            _selected_index = (_selected_index + 1) % _result_count;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && _result_count > 0) {
            _selected_index = (_selected_index == 0) ? _result_count - 1 : _selected_index - 1;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            execute_selected();
        }
    }
    ImGui::End();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);

    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow)) {
        close();
    }
}

void CommandPalette::draw_input() {
    Color bg_input = _theme ? _theme->get_background() : Color(0.08f, 0.08f, 0.1f);
    Color text = _theme ? _theme->get_text() : Color(0.9f, 0.9f, 0.92f);
    Color text_dim = _theme ? _theme->get_text_dim() : Color(0.5f, 0.5f, 0.52f);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 12.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(bg_input.r, bg_input.g, bg_input.b, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(text.r, text.g, text.b, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(text_dim.r, text_dim.g, text_dim.b, 1.0f));

    ImGui::SetNextItemWidth(WIDTH);

    if (_focus_input) {
        ImGui::SetKeyboardFocusHere();
        _focus_input = false;
    }

    bool changed = ImGui::InputTextWithHint("##palette_input", "Type a command...", 
                                            _search_buffer, sizeof(_search_buffer),
                                            ImGuiInputTextFlags_AutoSelectAll);
    if (changed) {
        update_search();
        _selected_index = 0;
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}

void CommandPalette::draw_results() {
    if (_result_count == 0) {
        return;
    }

    Color bg_hover = _theme ? _theme->get_accent().with_alpha(0.15f) : Color(0.3f, 0.5f, 0.8f, 0.15f);
    Color bg_selected = _theme ? _theme->get_accent().with_alpha(0.25f) : Color(0.3f, 0.5f, 0.8f, 0.25f);
    Color text = _theme ? _theme->get_text() : Color(0.9f, 0.9f, 0.92f);
    Color text_dim = _theme ? _theme->get_text_dim() : Color(0.5f, 0.5f, 0.52f);
    Color accent = _theme ? _theme->get_accent() : Color(0.3f, 0.5f, 0.8f);

    float results_height = _result_count * ITEM_HEIGHT;
    if (results_height > MAX_HEIGHT - 48.0f) {
        results_height = MAX_HEIGHT - 48.0f;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 4.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));

    if (ImGui::BeginChild("##results", ImVec2(WIDTH, results_height), false)) {
        for (uint32_t i = 0; i < _result_count; ++i) {
            Command* cmd = _registry->find_command(_results[i]);
            if (!cmd) continue;

            bool is_selected = (i == _selected_index);
            ImVec2 item_pos = ImGui::GetCursorScreenPos();

            if (is_selected) {
                ImGui::GetWindowDrawList()->AddRectFilled(
                    item_pos,
                    ImVec2(item_pos.x + WIDTH, item_pos.y + ITEM_HEIGHT),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(bg_selected.r, bg_selected.g, bg_selected.b, bg_selected.a))
                );
            }

            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(bg_hover.r, bg_hover.g, bg_hover.b, bg_hover.a));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(bg_selected.r, bg_selected.g, bg_selected.b, bg_selected.a));

            ImGui::PushID(static_cast<int>(i));
            if (ImGui::Selectable("##item", is_selected, 0, ImVec2(WIDTH, ITEM_HEIGHT))) {
                _selected_index = i;
                execute_selected();
            }
            ImGui::PopID();
            ImGui::PopStyleColor(3);

            ImGui::SetCursorScreenPos(ImVec2(item_pos.x + 16.0f, item_pos.y + (ITEM_HEIGHT - ImGui::GetTextLineHeight()) * 0.5f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(text.r, text.g, text.b, 1.0f));
            ImGui::TextUnformatted(cmd->get_name());
            ImGui::PopStyleColor();

            const char* shortcut = cmd->get_shortcut();
            if (shortcut && shortcut[0] != '\0') {
                float shortcut_width = ImGui::CalcTextSize(shortcut).x;
                ImGui::SetCursorScreenPos(ImVec2(item_pos.x + WIDTH - shortcut_width - 16.0f, 
                                                  item_pos.y + (ITEM_HEIGHT - ImGui::GetTextLineHeight()) * 0.5f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(text_dim.r, text_dim.g, text_dim.b, 1.0f));
                ImGui::TextUnformatted(shortcut);
                ImGui::PopStyleColor();
            }

            ImGui::SetCursorScreenPos(ImVec2(item_pos.x, item_pos.y + ITEM_HEIGHT));
        }
    }
    ImGui::EndChild();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

}
