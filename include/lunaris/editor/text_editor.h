#pragma once

#include <cstdint>
#include <cstddef>

namespace lunaris {

class Document;
class DocumentManager;
class FileOperations;
class Theme;
class TextBuffer;
struct UndoAction;

class TextEditor {
public:
    static constexpr float GUTTER_PADDING = 12.0f;
    static constexpr float LINE_HEIGHT_FACTOR = 1.4f;
    static constexpr float LEFT_MARGIN = 8.0f;
    static constexpr float TOP_MARGIN = 8.0f;

    TextEditor();
    ~TextEditor();

    void set_theme(Theme* theme) { _theme = theme; }
    void set_document(Document* doc);
    void set_document_manager(DocumentManager* mgr) { _doc_manager = mgr; }
    void set_file_operations(FileOperations* ops) { _file_ops = ops; }

    void on_ui();
    void focus() { _focus_requested = true; }

private:
    void handle_keyboard_input();
    void handle_mouse_input();
    void draw_gutter(float content_x, float content_y, float gutter_w, float height);
    void draw_text(float x, float y, float width, float height);
    void draw_cursor(float x, float y);
    void draw_selection(float x, float y, float width);

    void insert_text(const char* text, size_t len);
    void delete_range(size_t start, size_t end);
    void delete_char_before();
    void delete_char_after();

    void move_cursor(int delta, bool select);
    void move_cursor_to(size_t pos, bool select);
    void move_to_line_start(bool select);
    void move_to_line_end(bool select);
    void move_word_left(bool select);
    void move_word_right(bool select);
    void move_line_up(bool select);
    void move_line_down(bool select);

    void select_all();
    void copy_selection();
    void cut_selection();
    void paste();

    void undo();
    void redo();
    bool handle_text_undo(UndoAction* action);
    bool handle_text_redo(UndoAction* action);

    void ensure_cursor_visible();
    float get_gutter_width() const;
    float get_line_height() const;
    size_t pos_from_coords(float x, float y) const;
    void get_cursor_coords(float& x, float& y) const;
    bool is_word_char(char c) const;

    Document* _document;
    DocumentManager* _doc_manager;
    FileOperations* _file_ops;
    Theme* _theme;
    size_t _cursor_pos;
    size_t _selection_start;
    size_t _selection_end;
    float _scroll_x;
    float _scroll_y;
    float _content_width;
    float _content_height;
    bool _focused;
    bool _focus_requested;
    float _blink_timer;
    bool _cursor_visible;
    bool _dragging;
};

}

