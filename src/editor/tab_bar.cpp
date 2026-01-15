#include "lunaris/editor/tab_bar.h"
#include "lunaris/core/theme.h"
#include "lunaris/ui/components.h"
#include <imgui.h>
#include <tinyvk/assets/icons_font_awesome.h>
#include <cstring>

namespace lunaris {

TabBar::TabBar()
    : _tab_count(0)
    , _active_tab(INVALID_TAB_ID)
    , _next_id(1)
    , _theme(nullptr)
    , _on_tab_selected(nullptr)
    , _tab_selected_user_data(nullptr)
    , _on_tab_closed(nullptr)
    , _tab_closed_user_data(nullptr) {
    for (uint32_t i = 0; i < MAX_TABS; ++i) {
        _tabs[i].id = INVALID_TAB_ID;
        _tabs[i].info.title = nullptr;
        _tabs[i].info.filepath = nullptr;
        _tabs[i].info.modified = false;
        _tabs[i].info.pinned = false;
    }
}

TabBar::~TabBar() {
}

void TabBar::on_init() {
}

void TabBar::on_shutdown() {
}

TabID TabBar::generate_id() {
    return _next_id++;
}

TabID TabBar::add_tab(const TabInfo& info) {
    if (_tab_count >= MAX_TABS) {
        return INVALID_TAB_ID;
    }

    TabID id = generate_id();
    _tabs[_tab_count].id = id;
    _tabs[_tab_count].info = info;
    _tab_count++;

    if (_active_tab == INVALID_TAB_ID) {
        _active_tab = id;
    }

    return id;
}

void TabBar::remove_tab(TabID id) {
    for (uint32_t i = 0; i < _tab_count; ++i) {
        if (_tabs[i].id == id) {
            for (uint32_t j = i; j < _tab_count - 1; ++j) {
                _tabs[j] = _tabs[j + 1];
            }
            _tab_count--;

            if (_active_tab == id) {
                _active_tab = (_tab_count > 0) ? _tabs[0].id : INVALID_TAB_ID;
            }
            return;
        }
    }
}

void TabBar::set_active_tab(TabID id) {
    for (uint32_t i = 0; i < _tab_count; ++i) {
        if (_tabs[i].id == id) {
            _active_tab = id;
            return;
        }
    }
}

void TabBar::set_tab_modified(TabID id, bool modified) {
    for (uint32_t i = 0; i < _tab_count; ++i) {
        if (_tabs[i].id == id) {
            _tabs[i].info.modified = modified;
            return;
        }
    }
}

void TabBar::set_tab_title(TabID id, const char* title) {
    for (uint32_t i = 0; i < _tab_count; ++i) {
        if (_tabs[i].id == id) {
            _tabs[i].info.title = title;
            return;
        }
    }
}

void TabBar::on_ui() {
    if (_tab_count == 0) {
        return;
    }

    Color bg = _theme ? _theme->get_surface() : Color(0.12f, 0.12f, 0.14f);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(bg.r, bg.g, bg.b, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 0.0f));

    ImGui::BeginChild("##TabBar", ImVec2(0.0f, TAB_HEIGHT), false, ImGuiWindowFlags_NoScrollbar);
    for (uint32_t i = 0; i < _tab_count; ++i) {
        if (i > 0) {
            ImGui::SameLine(0.0f, 1.0f);
        }
        draw_tab(i);
    }
    ImGui::EndChild();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

void TabBar::draw_tab(uint32_t index) {
    const Tab& tab = _tabs[index];
    bool is_active = (_active_tab == tab.id);

    Color bg_active = _theme ? _theme->get_background() : Color(0.1f, 0.1f, 0.12f);
    Color bg_inactive = _theme ? _theme->get_surface().darken(0.02f) : Color(0.1f, 0.1f, 0.12f);
    Color accent = _theme ? _theme->get_accent() : Color(0.3f, 0.5f, 0.8f);
    Color text = _theme ? _theme->get_text() : Color(0.9f, 0.9f, 0.92f);
    Color text_dim = _theme ? _theme->get_text_dim() : Color(0.5f, 0.5f, 0.52f);

    ImVec4 bg_col = is_active
        ? ImVec4(bg_active.r, bg_active.g, bg_active.b, 1.0f)
        : ImVec4(bg_inactive.r, bg_inactive.g, bg_inactive.b, 1.0f);

    ImVec4 txt_col = is_active
        ? ImVec4(text.r, text.g, text.b, 1.0f)
        : ImVec4(text_dim.r, text_dim.g, text_dim.b, 1.0f);

    const char* title = tab.info.title ? tab.info.title : "Untitled";
    float font_size = ImGui::GetFontSize();
    float text_w = ImGui::CalcTextSize(title).x;
    float tab_w = text_w + font_size * 3.125f;
    if (tab_w < TAB_MIN_WIDTH) tab_w = TAB_MIN_WIDTH;
    if (tab_w > TAB_MAX_WIDTH) tab_w = TAB_MAX_WIDTH;

    ImGui::PushID(static_cast<int>(tab.id));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, bg_col);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(bg_col.x + 0.03f, bg_col.y + 0.03f, bg_col.z + 0.03f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, bg_col);

    ImVec2 pos = ImGui::GetCursorScreenPos();
    if (ImGui::Button("##tab", ImVec2(tab_w, TAB_HEIGHT))) {
        if (_active_tab != tab.id) {
            _active_tab = tab.id;
            if (_on_tab_selected) {
                _on_tab_selected(tab.id, _tab_selected_user_data);
            }
        }
    }

    float text_y = pos.y + (TAB_HEIGHT - ImGui::GetTextLineHeight()) * 0.5f;
    ImGui::GetWindowDrawList()->AddText(ImVec2(pos.x + font_size * 0.75f, text_y), ImGui::ColorConvertFloat4ToU32(txt_col), title);

    float close_size = font_size * 0.875f;
    float close_x = pos.x + tab_w - close_size - font_size * 0.5f;
    float close_y = pos.y + (TAB_HEIGHT - close_size) * 0.5f;
    ImVec2 close_min(close_x, close_y);
    ImVec2 close_max(close_x + close_size, close_y + close_size);

    bool close_hovered = ImGui::IsMouseHoveringRect(close_min, close_max);

    if (tab.info.modified && !close_hovered) {
        ImGui::GetWindowDrawList()->AddCircleFilled(
            ImVec2(close_x + close_size * 0.5f, close_y + close_size * 0.5f),
            font_size * 0.1875f,
            ImGui::ColorConvertFloat4ToU32(ImVec4(text_dim.r, text_dim.g, text_dim.b, 1.0f))
        );
    } else {
        ImU32 close_col = close_hovered
            ? ImGui::ColorConvertFloat4ToU32(ImVec4(text.r, text.g, text.b, 1.0f))
            : ImGui::ColorConvertFloat4ToU32(ImVec4(text_dim.r, text_dim.g, text_dim.b, 0.6f));

        if (is_active || close_hovered) {
            ImGui::GetWindowDrawList()->AddText(ImVec2(close_x + font_size * 0.125f, close_y), close_col, ICON_FA_XMARK);
        }
    }

    if (close_hovered && ImGui::IsMouseClicked(0)) {
        if (_on_tab_closed) {
            _on_tab_closed(tab.id, _tab_closed_user_data);
        }
    }

    if (is_active) {
        float indicator_h = font_size * 0.125f;
        ImGui::GetWindowDrawList()->AddRectFilled(
            ImVec2(pos.x, pos.y + TAB_HEIGHT - indicator_h),
            ImVec2(pos.x + tab_w, pos.y + TAB_HEIGHT),
            ImGui::ColorConvertFloat4ToU32(ImVec4(accent.r, accent.g, accent.b, 1.0f))
        );
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
    ImGui::PopID();
}

}
