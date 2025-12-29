#include "lunaris/editor/text_editor.h"
#include "lunaris/editor/document.h"
#include "lunaris/editor/document_manager.h"
#include "lunaris/editor/file_operations.h"
#include "lunaris/editor/undo_manager.h"
#include "lunaris/core/theme.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <cstring>
#include <cstdio>
#include <algorithm>

namespace lunaris {

TextEditor::TextEditor()
    : _document(nullptr)
    , _doc_manager(nullptr)
    , _file_ops(nullptr)
    , _theme(nullptr)
    , _cursor_pos(0)
    , _selection_start(0)
    , _selection_end(0)
    , _scroll_x(0.0f)
    , _scroll_y(0.0f)
    , _content_width(0.0f)
    , _content_height(0.0f)
    , _focused(false)
    , _focus_requested(false)
    , _blink_timer(0.0f)
    , _cursor_visible(true)
    , _dragging(false) {
}

TextEditor::~TextEditor() {
}

void TextEditor::set_document(Document* doc) {
    if (_document != doc) {
        _document = doc;
        _cursor_pos = 0;
        _selection_start = 0;
        _selection_end = 0;
        _scroll_x = 0.0f;
        _scroll_y = 0.0f;
    }
}

float TextEditor::get_gutter_width() const {
    if (!_document) return 60.0f;
    
    uint32_t line_count = _document->get_buffer()->get_line_count();
    int digits = 1;
    uint32_t n = line_count;
    while (n >= 10) {
        n /= 10;
        ++digits;
    }
    if (digits < 4) digits = 4;
    return ImGui::CalcTextSize("0").x * digits + GUTTER_PADDING * 2.0f;
}

float TextEditor::get_line_height() const {
    return ImGui::GetTextLineHeight() * LINE_HEIGHT_FACTOR;
}

void TextEditor::on_ui() {
    if (!_document) return;

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 content_pos = ImGui::GetCursorScreenPos();
    ImVec2 content_size = ImGui::GetContentRegionAvail();
    
    _content_width = content_size.x;
    _content_height = content_size.y;
    
    float gutter_w = get_gutter_width();
    float line_h = get_line_height();
    
    TextBuffer* buffer = _document->get_buffer();
    Color bg = _theme ? _theme->get_background() : Color(0.1f, 0.1f, 0.12f);
    Color gutter_bg = _theme ? _theme->get_background_alt() : Color(0.08f, 0.08f, 0.1f);
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    draw_list->AddRectFilled(content_pos,
        ImVec2(content_pos.x + content_size.x, content_pos.y + content_size.y),
        ImColor(bg.r, bg.g, bg.b, 1.0f));
    
    draw_list->AddRectFilled(content_pos,
        ImVec2(content_pos.x + gutter_w, content_pos.y + content_size.y),
        ImColor(gutter_bg.r, gutter_bg.g, gutter_bg.b, 1.0f));

    ImGui::InvisibleButton("##editor_area", content_size, ImGuiButtonFlags_MouseButtonLeft);
    bool hovered = ImGui::IsItemHovered();
    
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        _focused = true;
        _dragging = true;
    }
    
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        _dragging = false;
    }

    if (_focus_requested) {
        _focused = true;
        _focus_requested = false;
    }

    if (_focused) {
        _blink_timer += io.DeltaTime;
        if (_blink_timer >= 0.53f) {
            _blink_timer = 0.0f;
            _cursor_visible = !_cursor_visible;
        }
        handle_keyboard_input();
    }
    
    if (hovered || _dragging) {
        handle_mouse_input();
    }

    float text_x = content_pos.x + gutter_w + LEFT_MARGIN;
    float text_y = content_pos.y + TOP_MARGIN;
    
    draw_selection(text_x, text_y, content_size.x - gutter_w - LEFT_MARGIN);
    draw_text(text_x, text_y, content_size.x - gutter_w - LEFT_MARGIN, content_size.y);
    draw_gutter(content_pos.x, content_pos.y, gutter_w, content_size.y);
    
    if (_focused && _cursor_visible) {
        draw_cursor(text_x, text_y);
    }
}

void TextEditor::draw_gutter(float content_x, float content_y, float gutter_w, float height) {
    if (!_document) return;
    
    TextBuffer* buffer = _document->get_buffer();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    Color text_dim = _theme ? _theme->get_text_dim() : Color(0.5f, 0.5f, 0.55f);
    Color text_col = _theme ? _theme->get_text() : Color(0.9f, 0.9f, 0.92f);
    
    float line_h = get_line_height();
    float font_h = ImGui::GetTextLineHeight();
    float text_offset_y = (line_h - font_h) * 0.5f;
    
    uint32_t total_lines = buffer->get_line_count();
    uint32_t first_line = static_cast<uint32_t>(_scroll_y / line_h);
    uint32_t visible_lines = static_cast<uint32_t>(height / line_h) + 2;
    
    uint32_t cursor_line = buffer->get_line_at_pos(_cursor_pos);
    
    for (uint32_t i = 0; i < visible_lines && first_line + i < total_lines; ++i) {
        uint32_t line_idx = first_line + i;
        uint32_t line_num = line_idx + 1;
        float y = content_y + TOP_MARGIN + line_idx * line_h - _scroll_y + text_offset_y;
        
        if (y + line_h < content_y || y > content_y + height) continue;
        
        char line_str[16];
        snprintf(line_str, sizeof(line_str), "%u", line_num);
        float line_w = ImGui::CalcTextSize(line_str).x;
        float line_x = content_x + gutter_w - GUTTER_PADDING - line_w;
        
        ImU32 col = (line_idx == cursor_line) 
            ? ImColor(text_col.r, text_col.g, text_col.b, 1.0f)
            : ImColor(text_dim.r, text_dim.g, text_dim.b, 1.0f);
        
        draw_list->AddText(ImVec2(line_x, y), col, line_str);
    }
}

void TextEditor::draw_text(float x, float y, float width, float height) {
    if (!_document) return;
    
    TextBuffer* buffer = _document->get_buffer();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    Color text_col = _theme ? _theme->get_text() : Color(0.9f, 0.9f, 0.92f);
    
    float line_h = get_line_height();
    float font_h = ImGui::GetTextLineHeight();
    float text_offset_y = (line_h - font_h) * 0.5f;
    
    uint32_t total_lines = buffer->get_line_count();
    uint32_t first_line = static_cast<uint32_t>(_scroll_y / line_h);
    uint32_t visible_lines = static_cast<uint32_t>(height / line_h) + 2;
    
    const char* text = buffer->get_text();
    ImVec2 clip_min(x - LEFT_MARGIN, y - TOP_MARGIN);
    ImVec2 clip_max(x + width, y - TOP_MARGIN + height);
    draw_list->PushClipRect(clip_min, clip_max, true);
    
    for (uint32_t i = 0; i < visible_lines && first_line + i < total_lines; ++i) {
        uint32_t line_idx = first_line + i;
        size_t line_start = buffer->get_line_start(line_idx);
        size_t line_end = buffer->get_line_end(line_idx);
        
        float ly = y + line_idx * line_h - _scroll_y + text_offset_y;
        
        if (line_start < line_end) {
            draw_list->AddText(ImVec2(x - _scroll_x, ly),
                ImColor(text_col.r, text_col.g, text_col.b, 1.0f),
                text + line_start, text + line_end);
        }
    }
    
    draw_list->PopClipRect();
}

void TextEditor::draw_selection(float x, float y, float width) {
    if (!_document || _selection_start == _selection_end) return;
    
    TextBuffer* buffer = _document->get_buffer();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    Color accent = _theme ? _theme->get_accent() : Color(0.3f, 0.5f, 0.8f);
    
    size_t sel_start = std::min(_selection_start, _selection_end);
    size_t sel_end = std::max(_selection_start, _selection_end);
    
    uint32_t start_line = buffer->get_line_at_pos(sel_start);
    uint32_t end_line = buffer->get_line_at_pos(sel_end);
    
    float line_h = get_line_height();
    const char* text = buffer->get_text();
    
    for (uint32_t line = start_line; line <= end_line; ++line) {
        size_t line_start = buffer->get_line_start(line);
        size_t line_end = buffer->get_line_end(line);
        
        size_t sel_line_start = (line == start_line) ? sel_start : line_start;
        size_t sel_line_end = (line == end_line) ? sel_end : line_end;
        
        float start_x = x - _scroll_x;
        if (sel_line_start > line_start) {
            start_x += ImGui::CalcTextSize(text + line_start, text + sel_line_start).x;
        }
        
        float end_x = x - _scroll_x;
        if (sel_line_end > line_start) {
            end_x += ImGui::CalcTextSize(text + line_start, text + sel_line_end).x;
        }
        
        if (sel_line_end == line_end && line < end_line) {
            end_x += ImGui::CalcTextSize(" ").x;
        }
        
        float ly = y + line * line_h - _scroll_y;
        
        draw_list->AddRectFilled(
            ImVec2(start_x, ly),
            ImVec2(end_x, ly + line_h),
            ImColor(accent.r, accent.g, accent.b, 0.35f));
    }
}

void TextEditor::draw_cursor(float x, float y) {
    if (!_document) return;
    
    TextBuffer* buffer = _document->get_buffer();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    Color text_col = _theme ? _theme->get_text() : Color(0.9f, 0.9f, 0.92f);
    
    uint32_t cursor_line = buffer->get_line_at_pos(_cursor_pos);
    size_t line_start = buffer->get_line_start(cursor_line);
    
    float cursor_x = x - _scroll_x - 2.0f;
    if (_cursor_pos > line_start) {
        const char* text = buffer->get_text();
        cursor_x += ImGui::CalcTextSize(text + line_start, text + _cursor_pos).x;
    }
    
    float line_h = get_line_height();
    float font_h = ImGui::GetTextLineHeight();
    float text_offset_y = (line_h - font_h) * 0.5f;
    float cursor_y = y + cursor_line * line_h - _scroll_y + text_offset_y;
    
    draw_list->AddRectFilled(
        ImVec2(cursor_x, cursor_y - 1.0f),
        ImVec2(cursor_x + 2.0f, cursor_y + font_h + 1.0f),
        ImColor(text_col.r, text_col.g, text_col.b, 1.0f));
}

void TextEditor::handle_keyboard_input() {
    if (!_document || !_focused) return;
    
    ImGuiIO& io = ImGui::GetIO();
    bool ctrl = io.KeyCtrl || io.KeySuper;
    bool shift = io.KeyShift;
    
    for (int i = 0; i < io.InputQueueCharacters.Size; ++i) {
        ImWchar c = io.InputQueueCharacters[i];
        if (c >= 32 && c < 127) {
            if (_selection_start != _selection_end) {
                delete_range(std::min(_selection_start, _selection_end),
                           std::max(_selection_start, _selection_end));
            }
            char ch = static_cast<char>(c);
            insert_text(&ch, 1);
            _blink_timer = 0.0f;
            _cursor_visible = true;
        }
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
        if (_selection_start != _selection_end) {
            delete_range(std::min(_selection_start, _selection_end),
                       std::max(_selection_start, _selection_end));
        }
        insert_text("\n", 1);
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_Tab)) {
        if (_selection_start != _selection_end) {
            delete_range(std::min(_selection_start, _selection_end),
                       std::max(_selection_start, _selection_end));
        }
        insert_text("    ", 4);
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
        if (_selection_start != _selection_end) {
            delete_range(std::min(_selection_start, _selection_end),
                       std::max(_selection_start, _selection_end));
        } else {
            delete_char_before();
        }
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        if (_selection_start != _selection_end) {
            delete_range(std::min(_selection_start, _selection_end),
                       std::max(_selection_start, _selection_end));
        } else {
            delete_char_after();
        }
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
        if (ctrl) move_word_left(shift);
        else move_cursor(-1, shift);
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
        if (ctrl) move_word_right(shift);
        else move_cursor(1, shift);
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        move_line_up(shift);
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        move_line_down(shift);
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_Home)) {
        move_to_line_start(shift);
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_End)) {
        move_to_line_end(shift);
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }
    
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_A)) {
        select_all();
    }
    
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_C)) {
        copy_selection();
    }
    
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_X)) {
        cut_selection();
    }
    
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_V)) {
        paste();
    }
    
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Z)) {
        if (shift) redo();
        else undo();
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }
    
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Y)) {
        redo();
        _blink_timer = 0.0f;
        _cursor_visible = true;
    }
}

void TextEditor::handle_mouse_input() {
    if (!_document) return;
    
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 content_pos = ImGui::GetItemRectMin();
    float gutter_w = get_gutter_width();
    
    float text_x = content_pos.x + gutter_w + LEFT_MARGIN;
    float text_y = content_pos.y + TOP_MARGIN;
    
    if (io.MouseWheel != 0.0f) {
        float line_h = get_line_height();
        _scroll_y -= io.MouseWheel * line_h * 3.0f;
        if (_scroll_y < 0.0f) _scroll_y = 0.0f;
        
        TextBuffer* buffer = _document->get_buffer();
        float max_scroll = buffer->get_line_count() * line_h - _content_height + TOP_MARGIN * 2.0f;
        if (max_scroll < 0.0f) max_scroll = 0.0f;
        if (_scroll_y > max_scroll) _scroll_y = max_scroll;
    }
    
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ImVec2 mouse = io.MousePos;
        if (mouse.x > content_pos.x + gutter_w) {
            size_t pos = pos_from_coords(mouse.x - text_x + _scroll_x, mouse.y - text_y + _scroll_y);
            _cursor_pos = pos;
            _selection_start = pos;
            _selection_end = pos;
            _blink_timer = 0.0f;
            _cursor_visible = true;
        }
    }
    
    if (_dragging && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        ImVec2 mouse = io.MousePos;
        size_t pos = pos_from_coords(mouse.x - text_x + _scroll_x, mouse.y - text_y + _scroll_y);
        _cursor_pos = pos;
        _selection_end = pos;
    }
}

size_t TextEditor::pos_from_coords(float x, float y) const {
    if (!_document) return 0;
    
    TextBuffer* buffer = _document->get_buffer();
    float line_h = get_line_height();
    
    uint32_t line = 0;
    if (y > 0) {
        line = static_cast<uint32_t>(y / line_h);
    }
    if (line >= buffer->get_line_count()) {
        line = buffer->get_line_count() - 1;
    }
    
    size_t line_start = buffer->get_line_start(line);
    size_t line_end = buffer->get_line_end(line);
    
    if (x <= 0) return line_start;
    
    const char* text = buffer->get_text();
    
    for (size_t i = line_start; i < line_end; ++i) {
        float width_to_char = ImGui::CalcTextSize(text + line_start, text + i + 1).x;
        float width_before = (i > line_start) ? ImGui::CalcTextSize(text + line_start, text + i).x : 0.0f;
        float char_w = width_to_char - width_before;
        
        if (width_before + char_w * 0.5f > x) {
            return i;
        }
    }
    
    return line_end;
}

void TextEditor::insert_text(const char* text, size_t len) {
    if (!_document || len == 0) return;
    
    TextBuffer* buffer = _document->get_buffer();
    size_t old_cursor = _cursor_pos;
    
    buffer->insert(_cursor_pos, text, len, _cursor_pos);
    
    UndoManager::instance().record_text_insert(_document->get_id(), _document->get_filepath(), _cursor_pos, text, len, old_cursor, _cursor_pos + len);
    
    _cursor_pos += len;
    _selection_start = _cursor_pos;
    _selection_end = _cursor_pos;
    
    ensure_cursor_visible();
}

void TextEditor::delete_range(size_t start, size_t end) {
    if (!_document || start >= end) return;
    
    TextBuffer* buffer = _document->get_buffer();
    const char* text = buffer->get_text();
    size_t len = end - start;
    
    UndoManager::instance().record_text_delete(_document->get_id(), _document->get_filepath(), start, text + start, len, _cursor_pos, start);
    
    buffer->remove(start, len, _cursor_pos);
    
    _cursor_pos = start;
    _selection_start = start;
    _selection_end = start;
    
    ensure_cursor_visible();
}

void TextEditor::delete_char_before() {
    if (!_document || _cursor_pos == 0) return;
    delete_range(_cursor_pos - 1, _cursor_pos);
}

void TextEditor::delete_char_after() {
    if (!_document) return;
    TextBuffer* buffer = _document->get_buffer();
    if (_cursor_pos >= buffer->get_length()) return;
    delete_range(_cursor_pos, _cursor_pos + 1);
}

void TextEditor::move_cursor(int delta, bool select) {
    if (!_document) return;
    
    TextBuffer* buffer = _document->get_buffer();
    size_t len = buffer->get_length();
    
    size_t new_pos = _cursor_pos;
    if (delta < 0 && static_cast<size_t>(-delta) > _cursor_pos) {
        new_pos = 0;
    } else if (delta > 0 && _cursor_pos + delta > len) {
        new_pos = len;
    } else {
        new_pos = _cursor_pos + delta;
    }
    
    move_cursor_to(new_pos, select);
}

void TextEditor::move_cursor_to(size_t pos, bool select) {
    if (!select && _selection_start != _selection_end) {
        _selection_start = pos;
        _selection_end = pos;
    } else if (select) {
        if (_selection_start == _selection_end) {
            _selection_start = _cursor_pos;
        }
        _selection_end = pos;
    } else {
        _selection_start = pos;
        _selection_end = pos;
    }
    
    _cursor_pos = pos;
    ensure_cursor_visible();
}

void TextEditor::move_to_line_start(bool select) {
    if (!_document) return;
    TextBuffer* buffer = _document->get_buffer();
    uint32_t line = buffer->get_line_at_pos(_cursor_pos);
    move_cursor_to(buffer->get_line_start(line), select);
}

void TextEditor::move_to_line_end(bool select) {
    if (!_document) return;
    TextBuffer* buffer = _document->get_buffer();
    uint32_t line = buffer->get_line_at_pos(_cursor_pos);
    move_cursor_to(buffer->get_line_end(line), select);
}

bool TextEditor::is_word_char(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
}

void TextEditor::move_word_left(bool select) {
    if (!_document || _cursor_pos == 0) return;
    
    TextBuffer* buffer = _document->get_buffer();
    const char* text = buffer->get_text();
    size_t pos = _cursor_pos - 1;
    
    while (pos > 0 && !is_word_char(text[pos])) --pos;
    while (pos > 0 && is_word_char(text[pos - 1])) --pos;
    
    move_cursor_to(pos, select);
}

void TextEditor::move_word_right(bool select) {
    if (!_document) return;
    
    TextBuffer* buffer = _document->get_buffer();
    const char* text = buffer->get_text();
    size_t len = buffer->get_length();
    size_t pos = _cursor_pos;
    
    while (pos < len && is_word_char(text[pos])) ++pos;
    while (pos < len && !is_word_char(text[pos])) ++pos;
    
    move_cursor_to(pos, select);
}

void TextEditor::move_line_up(bool select) {
    if (!_document) return;
    
    TextBuffer* buffer = _document->get_buffer();
    uint32_t line = buffer->get_line_at_pos(_cursor_pos);
    if (line == 0) return;
    
    size_t col = buffer->get_column_at_pos(_cursor_pos);
    size_t new_pos = buffer->pos_from_line_col(line - 1, col);
    move_cursor_to(new_pos, select);
}

void TextEditor::move_line_down(bool select) {
    if (!_document) return;
    
    TextBuffer* buffer = _document->get_buffer();
    uint32_t line = buffer->get_line_at_pos(_cursor_pos);
    if (line >= buffer->get_line_count() - 1) return;
    
    size_t col = buffer->get_column_at_pos(_cursor_pos);
    size_t new_pos = buffer->pos_from_line_col(line + 1, col);
    move_cursor_to(new_pos, select);
}

void TextEditor::select_all() {
    if (!_document) return;
    TextBuffer* buffer = _document->get_buffer();
    _selection_start = 0;
    _selection_end = buffer->get_length();
    _cursor_pos = buffer->get_length();
}

void TextEditor::copy_selection() {
    if (!_document || _selection_start == _selection_end) return;
    
    TextBuffer* buffer = _document->get_buffer();
    const char* text = buffer->get_text();
    
    size_t start = std::min(_selection_start, _selection_end);
    size_t end = std::max(_selection_start, _selection_end);
    size_t len = end - start;
    
    char* clipboard = new char[len + 1];
    memcpy(clipboard, text + start, len);
    clipboard[len] = '\0';
    
    ImGui::SetClipboardText(clipboard);
    delete[] clipboard;
}

void TextEditor::cut_selection() {
    if (_selection_start == _selection_end) return;
    copy_selection();
    delete_range(std::min(_selection_start, _selection_end),
               std::max(_selection_start, _selection_end));
}

void TextEditor::paste() {
    const char* clipboard = ImGui::GetClipboardText();
    if (!clipboard || !_document) return;
    
    if (_selection_start != _selection_end) {
        delete_range(std::min(_selection_start, _selection_end),
                   std::max(_selection_start, _selection_end));
    }
    
    insert_text(clipboard, strlen(clipboard));
}

void TextEditor::undo() {
    UndoManager& mgr = UndoManager::instance();
    
    while (true) {
        const UndoAction* peek = mgr.peek_undo();
        if (!peek) return;
        
        if (mgr.is_file_action(peek->type)) {
            UndoAction* action = mgr.undo();
            if (_file_ops) {
                _file_ops->apply_undo(action);
            }
            return;
        }
        
        Document* target_doc = nullptr;
        if (_doc_manager) {
            target_doc = _doc_manager->get_document(peek->doc_id);
            if (!target_doc && peek->path) {
                DocumentID new_id = _doc_manager->open_document(peek->path);
                target_doc = _doc_manager->get_document(new_id);
            }
        } else if (_document && _document->get_id() == peek->doc_id) {
            target_doc = _document;
        }
        
        if (!target_doc) {
            mgr.undo();
            continue;
        }
        
        if (_doc_manager && target_doc != _document) {
            _doc_manager->set_active_document(target_doc->get_id());
            _document = target_doc;
            _cursor_pos = 0;
            _selection_start = 0;
            _selection_end = 0;
            _scroll_x = 0.0f;
            _scroll_y = 0.0f;
        }
        
        UndoAction* action = mgr.undo();
        if (!action) return;
        
        if (handle_text_undo(action)) return;
    }
}

void TextEditor::redo() {
    UndoManager& mgr = UndoManager::instance();
    
    while (true) {
        const UndoAction* peek = mgr.peek_redo();
        if (!peek) return;
        
        if (mgr.is_file_action(peek->type)) {
            UndoAction* action = mgr.redo();
            if (_file_ops) {
                _file_ops->apply_redo(action);
            }
            return;
        }
        
        Document* target_doc = nullptr;
        if (_doc_manager) {
            target_doc = _doc_manager->get_document(peek->doc_id);
            if (!target_doc && peek->path) {
                DocumentID new_id = _doc_manager->open_document(peek->path);
                target_doc = _doc_manager->get_document(new_id);
            }
        } else if (_document && _document->get_id() == peek->doc_id) {
            target_doc = _document;
        }
        
        if (!target_doc) {
            mgr.redo();
            continue;
        }
        
        if (_doc_manager && target_doc != _document) {
            _doc_manager->set_active_document(target_doc->get_id());
            _document = target_doc;
            _cursor_pos = 0;
            _selection_start = 0;
            _selection_end = 0;
            _scroll_x = 0.0f;
            _scroll_y = 0.0f;
        }
        
        UndoAction* action = mgr.redo();
        if (!action) return;
        
        if (handle_text_redo(action)) return;
    }
}

bool TextEditor::handle_text_undo(UndoAction* action) {
    if (!action || !_document) return false;
    
    TextBuffer* buffer = _document->get_buffer();
    
    if (action->type == UndoActionType::TextInsert) {
        buffer->remove_no_history(action->pos, action->len);
    } else if (action->type == UndoActionType::TextDelete) {
        buffer->insert_no_history(action->pos, action->text, action->len);
    } else {
        return false;
    }
    
    _cursor_pos = action->cursor_before;
    _selection_start = _cursor_pos;
    _selection_end = _cursor_pos;
    ensure_cursor_visible();
    return true;
}

bool TextEditor::handle_text_redo(UndoAction* action) {
    if (!action || !_document) return false;
    
    TextBuffer* buffer = _document->get_buffer();
    
    if (action->type == UndoActionType::TextInsert) {
        buffer->insert_no_history(action->pos, action->text, action->len);
    } else if (action->type == UndoActionType::TextDelete) {
        buffer->remove_no_history(action->pos, action->len);
    } else {
        return false;
    }
    
    _cursor_pos = action->cursor_after;
    _selection_start = _cursor_pos;
    _selection_end = _cursor_pos;
    ensure_cursor_visible();
    return true;
}

void TextEditor::ensure_cursor_visible() {
    if (!_document) return;
    
    TextBuffer* buffer = _document->get_buffer();
    float line_h = get_line_height();
    
    uint32_t cursor_line = buffer->get_line_at_pos(_cursor_pos);
    float cursor_y = cursor_line * line_h;
    
    if (cursor_y < _scroll_y) {
        _scroll_y = cursor_y;
    } else if (cursor_y + line_h > _scroll_y + _content_height - TOP_MARGIN * 2.0f) {
        _scroll_y = cursor_y + line_h - _content_height + TOP_MARGIN * 2.0f;
    }
    
    if (_scroll_y < 0.0f) _scroll_y = 0.0f;
}

void TextEditor::get_cursor_coords(float& x, float& y) const {
    if (!_document) {
        x = 0.0f;
        y = 0.0f;
        return;
    }
    
    TextBuffer* buffer = _document->get_buffer();
    uint32_t line = buffer->get_line_at_pos(_cursor_pos);
    size_t line_start = buffer->get_line_start(line);
    
    const char* text = buffer->get_text();
    x = 0.0f;
    if (_cursor_pos > line_start) {
        x = ImGui::CalcTextSize(text + line_start, text + _cursor_pos).x;
    }
    y = line * get_line_height();
}

}
