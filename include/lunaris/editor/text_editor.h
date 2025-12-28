#pragma once

#include <cstdint>
#include <cstddef>

namespace lunaris {

class Document;
class Theme;

class TextEditor {
public:
    static constexpr float LINE_HEIGHT_FACTOR = 1.4f;
    static constexpr float GUTTER_PADDING = 8.0f;
    static constexpr float LEFT_MARGIN = 4.0f;

    TextEditor();
    ~TextEditor();

    void set_theme(Theme* theme) { _theme = theme; }
    void set_document(Document* doc) { _document = doc; }

    void on_ui();

    void focus();
    bool is_focused() const { return _focused; }

    void insert_text(const char* text);
    void delete_selection();
    void select_all();
    void copy();
    void cut();
    void paste();
    void undo();
    void redo();

private:
    void handle_keyboard_input();
    void handle_mouse_input();

    void move_cursor_left(bool select);
    void move_cursor_right(bool select);
    void move_cursor_up(bool select);
    void move_cursor_down(bool select);
    void move_cursor_line_start(bool select);
    void move_cursor_line_end(bool select);
    void move_cursor_word_left(bool select);
    void move_cursor_word_right(bool select);

    void insert_char(char c);
    void insert_newline();
    void delete_char_before();
    void delete_char_after();

    void draw_background();
    void draw_gutter();
    void draw_selection();
    void draw_text();
    void draw_cursor();

    float get_line_height() const;
    float get_gutter_width() const;
    size_t pos_from_screen(float x, float y) const;

    bool is_word_char(char c) const;

    Document* _document;
    Theme* _theme;
    bool _focused;
    float _blink_timer;
    bool _cursor_visible;
    uint32_t _cached_version;
};

}
