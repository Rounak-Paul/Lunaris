#pragma once

#include "lunaris/editor/text_buffer.h"
#include "lunaris/editor/tab_bar.h"
#include <cstdint>

namespace lunaris {

using DocumentID = uint32_t;
constexpr DocumentID INVALID_DOCUMENT_ID = 0;

enum class DocumentType : uint8_t {
    PlainText,
    Cpp,
    Header,
    Python,
    JavaScript,
    TypeScript,
    Json,
    Xml,
    Markdown,
    CMake,
    Glsl,
    Unknown
};

class Document {
public:
    static constexpr size_t MAX_PATH_LENGTH = 512;
    static constexpr size_t MAX_TITLE_LENGTH = 64;

    Document();
    ~Document();

    bool open(const char* filepath);
    bool save();
    bool save_as(const char* filepath);
    void close();

    bool create_new(const char* title = nullptr);

    DocumentID get_id() const { return _id; }
    void set_id(DocumentID id) { _id = id; }

    TabID get_tab_id() const { return _tab_id; }
    void set_tab_id(TabID id) { _tab_id = id; }

    const char* get_filepath() const { return _filepath; }
    const char* get_title() const { return _title; }
    bool has_file() const { return _filepath[0] != '\0'; }

    TextBuffer* get_buffer() { return &_buffer; }
    const TextBuffer* get_buffer() const { return &_buffer; }

    bool is_modified() const { return _buffer.is_modified(); }
    bool is_new() const { return !has_file() && !is_modified(); }

    DocumentType get_type() const { return _type; }

    size_t get_cursor_pos() const { return _cursor_pos; }
    void set_cursor_pos(size_t pos) { _cursor_pos = pos; }

    size_t get_selection_start() const { return _selection_start; }
    size_t get_selection_end() const { return _selection_end; }
    void set_selection(size_t start, size_t end);
    void clear_selection();
    bool has_selection() const { return _selection_start != _selection_end; }

    float get_scroll_x() const { return _scroll_x; }
    float get_scroll_y() const { return _scroll_y; }
    void set_scroll(float x, float y) { _scroll_x = x; _scroll_y = y; }

private:
    void update_title_from_path();
    DocumentType detect_type_from_extension(const char* path);

    DocumentID _id;
    TabID _tab_id;
    char _filepath[MAX_PATH_LENGTH];
    char _title[MAX_TITLE_LENGTH];
    TextBuffer _buffer;
    DocumentType _type;
    size_t _cursor_pos;
    size_t _selection_start;
    size_t _selection_end;
    float _scroll_x;
    float _scroll_y;
};

}
