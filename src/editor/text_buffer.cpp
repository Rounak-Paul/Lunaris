#include "lunaris/editor/text_buffer.h"
#include <cstring>
#include <cstdio>

namespace lunaris {

TextBuffer::TextBuffer()
    : _data(nullptr)
    , _length(0)
    , _capacity(0)
    , _line_count(1)
    , _line_starts(nullptr)
    , _line_starts_capacity(0)
    , _modified(false)
    , _version(0) {
    ensure_capacity(INITIAL_CAPACITY);
    _line_starts_capacity = 1024;
    _line_starts = new size_t[_line_starts_capacity];
    _line_starts[0] = 0;
}

TextBuffer::~TextBuffer() {
    delete[] _data;
    delete[] _line_starts;
}

void TextBuffer::ensure_capacity(size_t required) {
    if (required <= _capacity) {
        return;
    }

    size_t new_capacity = _capacity == 0 ? INITIAL_CAPACITY : _capacity;
    while (new_capacity < required) {
        new_capacity *= 2;
    }

    char* new_data = new char[new_capacity];
    if (_data && _length > 0) {
        memcpy(new_data, _data, _length);
    }
    delete[] _data;
    _data = new_data;
    _capacity = new_capacity;
}

void TextBuffer::rebuild_line_starts() {
    _line_count = 1;
    _line_starts[0] = 0;

    for (size_t i = 0; i < _length; ++i) {
        if (_data[i] == '\n') {
            if (_line_count >= _line_starts_capacity) {
                size_t new_cap = _line_starts_capacity * 2;
                size_t* new_starts = new size_t[new_cap];
                memcpy(new_starts, _line_starts, _line_count * sizeof(size_t));
                delete[] _line_starts;
                _line_starts = new_starts;
                _line_starts_capacity = new_cap;
            }
            _line_starts[_line_count] = i + 1;
            ++_line_count;
        }
    }
}

bool TextBuffer::load_from_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        return false;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size < 0) {
        fclose(f);
        return false;
    }

    ensure_capacity(static_cast<size_t>(size) + 1);
    size_t read = fread(_data, 1, static_cast<size_t>(size), f);
    fclose(f);

    _length = read;
    _data[_length] = '\0';
    rebuild_line_starts();
    _modified = false;
    ++_version;
    return true;
}

bool TextBuffer::save_to_file(const char* path) {
    FILE* f = fopen(path, "wb");
    if (!f) {
        return false;
    }

    size_t written = fwrite(_data, 1, _length, f);
    fclose(f);

    if (written == _length) {
        _modified = false;
        return true;
    }
    return false;
}

void TextBuffer::set_text(const char* text, size_t length) {
    ensure_capacity(length + 1);
    memcpy(_data, text, length);
    _data[length] = '\0';
    _length = length;
    rebuild_line_starts();
    _modified = true;
    ++_version;
}

void TextBuffer::clear() {
    _length = 0;
    if (_data) {
        _data[0] = '\0';
    }
    _line_count = 1;
    _line_starts[0] = 0;
    _modified = false;
    ++_version;
}

void TextBuffer::insert(size_t pos, const char* text, size_t len) {
    if (pos > _length || len == 0) {
        return;
    }

    ensure_capacity(_length + len + 1);
    memmove(_data + pos + len, _data + pos, _length - pos);
    memcpy(_data + pos, text, len);
    _length += len;
    _data[_length] = '\0';
    rebuild_line_starts();
    _modified = true;
    ++_version;
}

void TextBuffer::remove(size_t pos, size_t len) {
    if (pos >= _length || len == 0) {
        return;
    }

    if (pos + len > _length) {
        len = _length - pos;
    }

    memmove(_data + pos, _data + pos + len, _length - pos - len);
    _length -= len;
    _data[_length] = '\0';
    rebuild_line_starts();
    _modified = true;
    ++_version;
}

char TextBuffer::char_at(size_t pos) const {
    if (pos >= _length) {
        return '\0';
    }
    return _data[pos];
}

size_t TextBuffer::get_line_start(uint32_t line) const {
    if (line >= _line_count) {
        return _length;
    }
    return _line_starts[line];
}

size_t TextBuffer::get_line_end(uint32_t line) const {
    if (line >= _line_count) {
        return _length;
    }
    if (line + 1 < _line_count) {
        size_t end = _line_starts[line + 1];
        if (end > 0 && _data[end - 1] == '\n') {
            return end - 1;
        }
        return end;
    }
    return _length;
}

uint32_t TextBuffer::get_line_at_pos(size_t pos) const {
    if (pos >= _length) {
        return _line_count > 0 ? _line_count - 1 : 0;
    }

    uint32_t low = 0;
    uint32_t high = _line_count;
    while (low < high) {
        uint32_t mid = (low + high) / 2;
        if (_line_starts[mid] <= pos) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    return low > 0 ? low - 1 : 0;
}

size_t TextBuffer::get_column_at_pos(size_t pos) const {
    uint32_t line = get_line_at_pos(pos);
    return pos - _line_starts[line];
}

size_t TextBuffer::pos_from_line_col(uint32_t line, size_t col) const {
    size_t line_start = get_line_start(line);
    size_t line_end = get_line_end(line);
    size_t max_col = line_end - line_start;
    if (col > max_col) {
        col = max_col;
    }
    return line_start + col;
}

}
