#include "lunaris/editor/text_editor.h"
#include "lunaris/editor/document.h"
#include "lunaris/core/theme.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace lunaris {

TextEditor::TextEditor()
    : _document(nullptr)
    , _theme(nullptr)
    , _focused(false)
    , _blink_timer(0.0f)
    , _cursor_visible(true)
    , _cached_version(0) {
}

TextEditor::~TextEditor() {
}

float TextEditor::get_line_height() const {
    return ImGui::GetTextLineHeight() * LINE_HEIGHT_FACTOR;
}

float TextEditor::get_gutter_width() const {
    if (!_document) {
        return 50.0f;
    }
    uint32_t line_count = _document->get_buffer()->get_line_count();
    int digits = 1;
    uint32_t n = line_count;
    while (n >= 10) {
        n /= 10;
        ++digits;
    }
    if (digits < 3) digits = 3;
    return ImGui::CalcTextSize("0").x * digits + GUTTER_PADDING * 2.0f;
}

void TextEditor::on_ui() {
    if (!_document) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 content_pos = ImGui::GetCursorScreenPos();
    ImVec2 content_size = ImGui::GetContentRegionAvail();

    ImGui::PushID("TextEditor");

    ImGui::InvisibleButton("##editor_area", content_size, ImGuiButtonFlags_AllowOverlap);
    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemActive() && ImGui::IsMouseClicked(ImGuiMouseButton_Left);

    if (clicked) {
        _focused = true;
    }

    if (_focused) {
        _blink_timer += io.DeltaTime;
        if (_blink_timer >= 0.5f) {
            _blink_timer = 0.0f;
            _cursor_visible = !_cursor_visible;
        }

        handle_keyboard_input();
    }

    if (hovered || _focused) {
        handle_mouse_input();
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    float gutter_w = get_gutter_width();
    float line_h = get_line_height();
    TextBuffer* buffer = _document->get_buffer();

    Color bg = _theme ? _theme->get_background() : Color(0.1f, 0.1f, 0.12f);
    Color gutter_bg = _theme ? _theme->get_background_alt() : Color(0.08f, 0.08f, 0.1f);
    Color text_col = _theme ? _theme->get_text() : Color(0.9f, 0.9f, 0.92f);
    Color text_dim = _theme ? _theme->get_text_dim() : Color(0.5f, 0.5f, 0.55f);
    Color accent = _theme ? _theme->get_accent() : Color(0.3f, 0.5f, 0.8f);

    draw_list->AddRectFilled(content_pos,
        ImVec2(content_pos.x + content_size.x, content_pos.y + content_size.y),
        ImColor(bg.r, bg.g, bg.b, 1.0f));

    draw_list->AddRectFilled(content_pos,
        ImVec2(content_pos.x + gutter_w, content_pos.y + content_size.y),
        ImColor(gutter_bg.r, gutter_bg.g, gutter_bg.b, 1.0f));

    float scroll_y = _document->get_scroll_y();
    uint32_t first_line = static_cast<uint32_t>(scroll_y / line_h);
    uint32_t visible_lines = static_cast<uint32_t>(content_size.y / line_h) + 2;
    uint32_t total_lines = buffer->get_line_count();

    size_t cursor_pos = _document->get_cursor_pos();
    uint32_t cursor_line = buffer->get_line_at_pos(cursor_pos);
    size_t cursor_col = buffer->get_column_at_pos(cursor_pos);

    float cursor_y = content_pos.y + cursor_line * line_h - scroll_y;
    draw_list->AddRectFilled(
        ImVec2(content_pos.x + gutter_w, cursor_y),
        ImVec2(content_pos.x + content_size.x, cursor_y + line_h),
        ImColor(accent.r, accent.g, accent.b, 0.1f));

    size_t sel_start = _document->get_selection_start();
    size_t sel_end = _document->get_selection_end();
    if (sel_start != sel_end) {
        if (sel_start > sel_end) {
            size_t tmp = sel_start;
            sel_start = sel_end;
            sel_end = tmp;
        }

        uint32_t sel_start_line = buffer->get_line_at_pos(sel_start);
        uint32_t sel_end_line = buffer->get_line_at_pos(sel_end);

        for (uint32_t line = sel_start_line; line <= sel_end_line && line < total_lines; ++line) {
            size_t line_start = buffer->get_line_start(line);
            size_t line_end = buffer->get_line_end(line);

            size_t start_col = (line == sel_start_line) ? (sel_start - line_start) : 0;
            size_t end_col = (line == sel_end_line) ? (sel_end - line_start) : (line_end - line_start);

            float sel_x1 = content_pos.x + gutter_w + LEFT_MARGIN + start_col * ImGui::CalcTextSize("0").x;
            float sel_x2 = content_pos.x + gutter_w + LEFT_MARGIN + end_col * ImGui::CalcTextSize("0").x;
            float sel_y = content_pos.y + line * line_h - scroll_y;

            draw_list->AddRectFilled(
                ImVec2(sel_x1, sel_y),
                ImVec2(sel_x2, sel_y + line_h),
                ImColor(accent.r, accent.g, accent.b, 0.3f));
        }
    }

    for (uint32_t i = 0; i < visible_lines && first_line + i < total_lines; ++i) {
        uint32_t line_num = first_line + i + 1;
        float y = content_pos.y + (first_line + i) * line_h - scroll_y;
        float text_y = y + (line_h - ImGui::GetTextLineHeight()) * 0.5f;

        char line_str[16];
        snprintf(line_str, sizeof(line_str), "%u", line_num);
        float line_w = ImGui::CalcTextSize(line_str).x;
        float line_x = content_pos.x + gutter_w - GUTTER_PADDING - line_w;

        ImU32 line_col = (first_line + i == cursor_line)
            ? ImColor(text_col.r, text_col.g, text_col.b, 1.0f)
            : ImColor(text_dim.r, text_dim.g, text_dim.b, 1.0f);

        draw_list->AddText(ImVec2(line_x, text_y), line_col, line_str);
    }

    const char* text = buffer->get_text();
    float char_w = ImGui::CalcTextSize("0").x;

    for (uint32_t i = 0; i < visible_lines && first_line + i < total_lines; ++i) {
        uint32_t line_idx = first_line + i;
        size_t line_start = buffer->get_line_start(line_idx);
        size_t line_end = buffer->get_line_end(line_idx);

        float y = content_pos.y + line_idx * line_h - scroll_y;
        float text_y = y + (line_h - ImGui::GetTextLineHeight()) * 0.5f;
        float text_x = content_pos.x + gutter_w + LEFT_MARGIN;

        if (line_start < line_end) {
            draw_list->AddText(ImVec2(text_x, text_y),
                ImColor(text_col.r, text_col.g, text_col.b, 1.0f),
                text + line_start, text + line_end);
        }
    }

    if (_focused && _cursor_visible) {
        float cursor_x = content_pos.x + gutter_w + LEFT_MARGIN + cursor_col * char_w;
        float cy = content_pos.y + cursor_line * line_h - scroll_y;

        draw_list->AddRectFilled(
            ImVec2(cursor_x, cy + 2.0f),
            ImVec2(cursor_x + 2.0f, cy + line_h - 2.0f),
            ImColor(text_col.r, text_col.g, text_col.b, 1.0f));
    }

    ImGui::PopID();
}

void TextEditor::handle_keyboard_input() {
    if (!_document || !_focused) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    bool ctrl = io.KeyCtrl || io.KeySuper;
    bool shift = io.KeyShift;

    if (io.InputQueueCharacters.Size > 0) {
        for (int i = 0; i < io.InputQueueCharacters.Size; ++i) {
            ImWchar c = io.InputQueueCharacters[i];
            if (c >= 32 && c < 127) {
                if (_document->has_selection()) {
                    delete_selection();
                }
                insert_char(static_cast<char>(c));
            }
        }
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
        if (_document->has_selection()) {
            delete_selection();
        }
        insert_newline();
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Tab)) {
        if (_document->has_selection()) {
            delete_selection();
        }
        insert_text("    ");
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
        if (_document->has_selection()) {
            delete_selection();
        } else {
            delete_char_before();
        }
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        if (_document->has_selection()) {
            delete_selection();
        } else {
            delete_char_after();
        }
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
        if (ctrl) {
            move_cursor_word_left(shift);
        } else {
            move_cursor_left(shift);
        }
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
        if (ctrl) {
            move_cursor_word_right(shift);
        } else {
            move_cursor_right(shift);
        }
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        move_cursor_up(shift);
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        move_cursor_down(shift);
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Home)) {
        move_cursor_line_start(shift);
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_End)) {
        move_cursor_line_end(shift);
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }

    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_A)) {
        select_all();
    }

    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_C)) {
        copy();
    }

    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_X)) {
        cut();
    }

    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_V)) {
        paste();
    }
}

void TextEditor::handle_mouse_input() {
    if (!_document) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();

    if (io.MouseWheel != 0.0f) {
        float scroll_y = _document->get_scroll_y();
        scroll_y -= io.MouseWheel * get_line_height() * 3.0f;
        if (scroll_y < 0.0f) scroll_y = 0.0f;

        float max_scroll = _document->get_buffer()->get_line_count() * get_line_height() - 100.0f;
        if (max_scroll < 0.0f) max_scroll = 0.0f;
        if (scroll_y > max_scroll) scroll_y = max_scroll;

        _document->set_scroll(_document->get_scroll_x(), scroll_y);
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ImVec2 mouse_pos = io.MousePos;
        ImVec2 content_pos = ImGui::GetItemRectMin();
        float gutter_w = get_gutter_width();

        if (mouse_pos.x > content_pos.x + gutter_w) {
            size_t pos = pos_from_screen(mouse_pos.x, mouse_pos.y);
            _document->set_cursor_pos(pos);
            _document->clear_selection();
            _blink_timer = 0.0f;
            _cursor_visible = true;
        }
    }

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        ImVec2 mouse_pos = io.MousePos;
        size_t pos = pos_from_screen(mouse_pos.x, mouse_pos.y);

        size_t sel_start = _document->get_selection_start();
        if (sel_start == _document->get_selection_end()) {
            sel_start = _document->get_cursor_pos();
        }
        _document->set_selection(sel_start, pos);
        _document->set_cursor_pos(pos);
    }
}

size_t TextEditor::pos_from_screen(float x, float y) const {
    if (!_document) {
        return 0;
    }

    TextBuffer* buffer = _document->get_buffer();
    ImVec2 content_pos = ImGui::GetItemRectMin();
    float gutter_w = get_gutter_width();
    float line_h = get_line_height();
    float scroll_y = _document->get_scroll_y();
    float char_w = ImGui::CalcTextSize("0").x;

    float rel_y = y - content_pos.y + scroll_y;
    uint32_t line = static_cast<uint32_t>(rel_y / line_h);
    if (line >= buffer->get_line_count()) {
        line = buffer->get_line_count() - 1;
    }

    float rel_x = x - content_pos.x - gutter_w - LEFT_MARGIN;
    size_t col = 0;
    if (rel_x > 0) {
        col = static_cast<size_t>(rel_x / char_w + 0.5f);
    }

    return buffer->pos_from_line_col(line, col);
}

void TextEditor::focus() {
    _focused = true;
    _blink_timer = 0.0f;
    _cursor_visible = true;
}

void TextEditor::insert_text(const char* text) {
    if (!_document || !text) {
        return;
    }

    size_t len = strlen(text);
    if (len == 0) {
        return;
    }

    TextBuffer* buffer = _document->get_buffer();
    size_t pos = _document->get_cursor_pos();
    buffer->insert(pos, text, len);
    _document->set_cursor_pos(pos + len);
    _document->clear_selection();
}

void TextEditor::insert_char(char c) {
    char buf[2] = {c, '\0'};
    insert_text(buf);
}

void TextEditor::insert_newline() {
    insert_text("\n");
}

void TextEditor::delete_selection() {
    if (!_document || !_document->has_selection()) {
        return;
    }

    size_t start = _document->get_selection_start();
    size_t end = _document->get_selection_end();
    if (start > end) {
        size_t tmp = start;
        start = end;
        end = tmp;
    }

    TextBuffer* buffer = _document->get_buffer();
    buffer->remove(start, end - start);
    _document->set_cursor_pos(start);
    _document->clear_selection();
}

void TextEditor::delete_char_before() {
    if (!_document) {
        return;
    }

    size_t pos = _document->get_cursor_pos();
    if (pos == 0) {
        return;
    }

    TextBuffer* buffer = _document->get_buffer();
    buffer->remove(pos - 1, 1);
    _document->set_cursor_pos(pos - 1);
}

void TextEditor::delete_char_after() {
    if (!_document) {
        return;
    }

    size_t pos = _document->get_cursor_pos();
    TextBuffer* buffer = _document->get_buffer();
    if (pos >= buffer->get_length()) {
        return;
    }

    buffer->remove(pos, 1);
}

void TextEditor::select_all() {
    if (!_document) {
        return;
    }

    TextBuffer* buffer = _document->get_buffer();
    _document->set_selection(0, buffer->get_length());
    _document->set_cursor_pos(buffer->get_length());
}

void TextEditor::copy() {
    if (!_document || !_document->has_selection()) {
        return;
    }

    size_t start = _document->get_selection_start();
    size_t end = _document->get_selection_end();
    if (start > end) {
        size_t tmp = start;
        start = end;
        end = tmp;
    }

    TextBuffer* buffer = _document->get_buffer();
    const char* text = buffer->get_text();

    size_t len = end - start;
    char* clipboard = new char[len + 1];
    memcpy(clipboard, text + start, len);
    clipboard[len] = '\0';

    ImGui::SetClipboardText(clipboard);
    delete[] clipboard;
}

void TextEditor::cut() {
    if (!_document || !_document->has_selection()) {
        return;
    }

    copy();
    delete_selection();
}

void TextEditor::paste() {
    const char* clipboard = ImGui::GetClipboardText();
    if (!clipboard || !_document) {
        return;
    }

    if (_document->has_selection()) {
        delete_selection();
    }

    insert_text(clipboard);
}

void TextEditor::undo() {
}

void TextEditor::redo() {
}

void TextEditor::move_cursor_left(bool select) {
    if (!_document) {
        return;
    }

    size_t pos = _document->get_cursor_pos();
    if (pos > 0) {
        size_t new_pos = pos - 1;
        if (select) {
            size_t sel_start = _document->get_selection_start();
            size_t sel_end = _document->get_selection_end();
            if (sel_start == sel_end) {
                sel_start = pos;
            }
            _document->set_selection(sel_start, new_pos);
        } else {
            _document->clear_selection();
        }
        _document->set_cursor_pos(new_pos);
    }
}

void TextEditor::move_cursor_right(bool select) {
    if (!_document) {
        return;
    }

    TextBuffer* buffer = _document->get_buffer();
    size_t pos = _document->get_cursor_pos();
    size_t len = buffer->get_length();

    if (pos < len) {
        size_t new_pos = pos + 1;
        if (select) {
            size_t sel_start = _document->get_selection_start();
            size_t sel_end = _document->get_selection_end();
            if (sel_start == sel_end) {
                sel_start = pos;
            }
            _document->set_selection(sel_start, new_pos);
        } else {
            _document->clear_selection();
        }
        _document->set_cursor_pos(new_pos);
    }
}

void TextEditor::move_cursor_up(bool select) {
    if (!_document) {
        return;
    }

    TextBuffer* buffer = _document->get_buffer();
    size_t pos = _document->get_cursor_pos();
    uint32_t line = buffer->get_line_at_pos(pos);

    if (line > 0) {
        size_t col = buffer->get_column_at_pos(pos);
        size_t new_pos = buffer->pos_from_line_col(line - 1, col);

        if (select) {
            size_t sel_start = _document->get_selection_start();
            size_t sel_end = _document->get_selection_end();
            if (sel_start == sel_end) {
                sel_start = pos;
            }
            _document->set_selection(sel_start, new_pos);
        } else {
            _document->clear_selection();
        }
        _document->set_cursor_pos(new_pos);
    }
}

void TextEditor::move_cursor_down(bool select) {
    if (!_document) {
        return;
    }

    TextBuffer* buffer = _document->get_buffer();
    size_t pos = _document->get_cursor_pos();
    uint32_t line = buffer->get_line_at_pos(pos);

    if (line < buffer->get_line_count() - 1) {
        size_t col = buffer->get_column_at_pos(pos);
        size_t new_pos = buffer->pos_from_line_col(line + 1, col);

        if (select) {
            size_t sel_start = _document->get_selection_start();
            size_t sel_end = _document->get_selection_end();
            if (sel_start == sel_end) {
                sel_start = pos;
            }
            _document->set_selection(sel_start, new_pos);
        } else {
            _document->clear_selection();
        }
        _document->set_cursor_pos(new_pos);
    }
}

void TextEditor::move_cursor_line_start(bool select) {
    if (!_document) {
        return;
    }

    TextBuffer* buffer = _document->get_buffer();
    size_t pos = _document->get_cursor_pos();
    uint32_t line = buffer->get_line_at_pos(pos);
    size_t new_pos = buffer->get_line_start(line);

    if (select) {
        size_t sel_start = _document->get_selection_start();
        size_t sel_end = _document->get_selection_end();
        if (sel_start == sel_end) {
            sel_start = pos;
        }
        _document->set_selection(sel_start, new_pos);
    } else {
        _document->clear_selection();
    }
    _document->set_cursor_pos(new_pos);
}

void TextEditor::move_cursor_line_end(bool select) {
    if (!_document) {
        return;
    }

    TextBuffer* buffer = _document->get_buffer();
    size_t pos = _document->get_cursor_pos();
    uint32_t line = buffer->get_line_at_pos(pos);
    size_t new_pos = buffer->get_line_end(line);

    if (select) {
        size_t sel_start = _document->get_selection_start();
        size_t sel_end = _document->get_selection_end();
        if (sel_start == sel_end) {
            sel_start = pos;
        }
        _document->set_selection(sel_start, new_pos);
    } else {
        _document->clear_selection();
    }
    _document->set_cursor_pos(new_pos);
}

bool TextEditor::is_word_char(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
}

void TextEditor::move_cursor_word_left(bool select) {
    if (!_document) {
        return;
    }

    TextBuffer* buffer = _document->get_buffer();
    size_t pos = _document->get_cursor_pos();
    const char* text = buffer->get_text();

    if (pos == 0) {
        return;
    }

    size_t new_pos = pos - 1;
    while (new_pos > 0 && !is_word_char(text[new_pos])) {
        --new_pos;
    }
    while (new_pos > 0 && is_word_char(text[new_pos - 1])) {
        --new_pos;
    }

    if (select) {
        size_t sel_start = _document->get_selection_start();
        size_t sel_end = _document->get_selection_end();
        if (sel_start == sel_end) {
            sel_start = pos;
        }
        _document->set_selection(sel_start, new_pos);
    } else {
        _document->clear_selection();
    }
    _document->set_cursor_pos(new_pos);
}

void TextEditor::move_cursor_word_right(bool select) {
    if (!_document) {
        return;
    }

    TextBuffer* buffer = _document->get_buffer();
    size_t pos = _document->get_cursor_pos();
    size_t len = buffer->get_length();
    const char* text = buffer->get_text();

    if (pos >= len) {
        return;
    }

    size_t new_pos = pos;
    while (new_pos < len && is_word_char(text[new_pos])) {
        ++new_pos;
    }
    while (new_pos < len && !is_word_char(text[new_pos])) {
        ++new_pos;
    }

    if (select) {
        size_t sel_start = _document->get_selection_start();
        size_t sel_end = _document->get_selection_end();
        if (sel_start == sel_end) {
            sel_start = pos;
        }
        _document->set_selection(sel_start, new_pos);
    } else {
        _document->clear_selection();
    }
    _document->set_cursor_pos(new_pos);
}

}
