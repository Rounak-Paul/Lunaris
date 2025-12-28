#pragma once

#include <cstdint>
#include <cstddef>

namespace lunaris {

class TextBuffer {
public:
    static constexpr size_t INITIAL_CAPACITY = 4096;
    static constexpr size_t MAX_LINE_COUNT = 1000000;

    TextBuffer();
    ~TextBuffer();

    bool load_from_file(const char* path);
    bool save_to_file(const char* path);
    void set_text(const char* text, size_t length);
    void clear();

    const char* get_text() const { return _data; }
    size_t get_length() const { return _length; }
    uint32_t get_line_count() const { return _line_count; }

    void insert(size_t pos, const char* text, size_t len);
    void remove(size_t pos, size_t len);
    char char_at(size_t pos) const;

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

    char* _data;
    size_t _length;
    size_t _capacity;
    uint32_t _line_count;
    size_t* _line_starts;
    size_t _line_starts_capacity;
    bool _modified;
    uint32_t _version;
};

}
