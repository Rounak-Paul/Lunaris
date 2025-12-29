#pragma once

#include <cstdint>
#include <cstddef>

namespace lunaris {

enum class EditType : uint8_t {
    Insert,
    Remove
};

struct EditOperation {
    EditType type;
    size_t pos;
    char* text;
    size_t len;
    size_t cursor_before;
    size_t cursor_after;
};

class TextBuffer {
public:
    static constexpr size_t INITIAL_CAPACITY = 4096;
    static constexpr size_t MAX_LINE_COUNT = 1000000;
    static constexpr size_t MAX_UNDO_HISTORY = 1000;

    TextBuffer();
    ~TextBuffer();

    bool load_from_file(const char* path);
    bool save_to_file(const char* path);
    void set_text(const char* text, size_t length);
    void clear();

    const char* get_text() const { return _data; }
    size_t get_length() const { return _length; }
    uint32_t get_line_count() const { return _line_count; }

    void insert(size_t pos, const char* text, size_t len, size_t cursor_pos);
    void remove(size_t pos, size_t len, size_t cursor_pos);
    void insert_no_history(size_t pos, const char* text, size_t len);
    void remove_no_history(size_t pos, size_t len);
    char char_at(size_t pos) const;

    bool can_undo() const { return _undo_count > 0; }
    bool can_redo() const { return _redo_count > 0; }
    size_t undo();
    size_t redo();
    void clear_history();

    size_t get_line_start(uint32_t line) const;
    size_t get_line_end(uint32_t line) const;
    uint32_t get_line_at_pos(size_t pos) const;
    size_t get_column_at_pos(size_t pos) const;
    size_t pos_from_line_col(uint32_t line, size_t col) const;

    bool is_modified() const { return _modified; }
    void set_modified(bool modified) { _modified = modified; }

    uint32_t get_version() const { return _version; }

private:
    void ensure_capacity(size_t required);
    void rebuild_line_starts();
    void insert_raw(size_t pos, const char* text, size_t len);
    void remove_raw(size_t pos, size_t len);
    void push_undo(EditOperation op);
    void push_redo(EditOperation op);
    void free_operation(EditOperation& op);
    void clear_redo();

    char* _data;
    size_t _length;
    size_t _capacity;
    uint32_t _line_count;
    size_t* _line_starts;
    size_t _line_starts_capacity;
    bool _modified;
    uint32_t _version;

    EditOperation* _undo_stack;
    size_t _undo_count;
    EditOperation* _redo_stack;
    size_t _redo_count;
};

}
