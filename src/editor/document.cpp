#include "lunaris/editor/document.h"
#include <cstring>
#include <cstdio>

namespace lunaris {

static uint32_t s_untitled_counter = 0;

Document::Document()
    : _id(INVALID_DOCUMENT_ID)
    , _tab_id(INVALID_TAB_ID)
    , _type(DocumentType::PlainText)
    , _cursor_pos(0)
    , _selection_start(0)
    , _selection_end(0)
    , _scroll_x(0.0f)
    , _scroll_y(0.0f) {
    _filepath[0] = '\0';
    _title[0] = '\0';
}

Document::~Document() {
    close();
}

bool Document::open(const char* filepath) {
    if (!_buffer.load_from_file(filepath)) {
        return false;
    }

    size_t len = strlen(filepath);
    if (len >= MAX_PATH_LENGTH) {
        len = MAX_PATH_LENGTH - 1;
    }
    memcpy(_filepath, filepath, len);
    _filepath[len] = '\0';

    update_title_from_path();
    _type = detect_type_from_extension(filepath);
    _cursor_pos = 0;
    _selection_start = 0;
    _selection_end = 0;
    _scroll_x = 0.0f;
    _scroll_y = 0.0f;
    return true;
}

bool Document::save() {
    if (!has_file()) {
        return false;
    }
    return _buffer.save_to_file(_filepath);
}

bool Document::save_as(const char* filepath) {
    if (!_buffer.save_to_file(filepath)) {
        return false;
    }

    size_t len = strlen(filepath);
    if (len >= MAX_PATH_LENGTH) {
        len = MAX_PATH_LENGTH - 1;
    }
    memcpy(_filepath, filepath, len);
    _filepath[len] = '\0';

    update_title_from_path();
    _type = detect_type_from_extension(filepath);
    return true;
}

void Document::close() {
    _buffer.clear();
    _filepath[0] = '\0';
    _title[0] = '\0';
    _cursor_pos = 0;
    _selection_start = 0;
    _selection_end = 0;
}

bool Document::create_new(const char* title) {
    _buffer.clear();
    _filepath[0] = '\0';

    if (title) {
        size_t len = strlen(title);
        if (len >= MAX_TITLE_LENGTH) {
            len = MAX_TITLE_LENGTH - 1;
        }
        memcpy(_title, title, len);
        _title[len] = '\0';
    } else {
        ++s_untitled_counter;
        snprintf(_title, MAX_TITLE_LENGTH, "Untitled-%u", s_untitled_counter);
    }

    _type = DocumentType::PlainText;
    _cursor_pos = 0;
    _selection_start = 0;
    _selection_end = 0;
    _scroll_x = 0.0f;
    _scroll_y = 0.0f;
    return true;
}

void Document::set_selection(size_t start, size_t end) {
    size_t len = _buffer.get_length();
    if (start > len) start = len;
    if (end > len) end = len;
    _selection_start = start;
    _selection_end = end;
}

void Document::clear_selection() {
    _selection_start = _cursor_pos;
    _selection_end = _cursor_pos;
}

void Document::update_title_from_path() {
    const char* name = _filepath;
    const char* last_sep = nullptr;

    for (const char* p = _filepath; *p; ++p) {
        if (*p == '/' || *p == '\\') {
            last_sep = p;
        }
    }

    if (last_sep) {
        name = last_sep + 1;
    }

    size_t len = strlen(name);
    if (len >= MAX_TITLE_LENGTH) {
        len = MAX_TITLE_LENGTH - 1;
    }
    memcpy(_title, name, len);
    _title[len] = '\0';
}

DocumentType Document::detect_type_from_extension(const char* path) {
    const char* ext = nullptr;
    for (const char* p = path; *p; ++p) {
        if (*p == '.') {
            ext = p;
        }
    }

    if (!ext) {
        return DocumentType::PlainText;
    }

    auto match = [](const char* a, const char* b) -> bool {
        while (*a && *b) {
            char ca = *a >= 'A' && *a <= 'Z' ? *a + 32 : *a;
            char cb = *b >= 'A' && *b <= 'Z' ? *b + 32 : *b;
            if (ca != cb) return false;
            ++a;
            ++b;
        }
        return *a == *b;
    };

    if (match(ext, ".cpp") || match(ext, ".cc") || match(ext, ".cxx") || match(ext, ".c")) {
        return DocumentType::Cpp;
    }
    if (match(ext, ".h") || match(ext, ".hpp") || match(ext, ".hxx")) {
        return DocumentType::Header;
    }
    if (match(ext, ".py")) {
        return DocumentType::Python;
    }
    if (match(ext, ".js")) {
        return DocumentType::JavaScript;
    }
    if (match(ext, ".ts") || match(ext, ".tsx")) {
        return DocumentType::TypeScript;
    }
    if (match(ext, ".json")) {
        return DocumentType::Json;
    }
    if (match(ext, ".xml") || match(ext, ".html") || match(ext, ".htm")) {
        return DocumentType::Xml;
    }
    if (match(ext, ".md") || match(ext, ".markdown")) {
        return DocumentType::Markdown;
    }
    if (match(ext, ".cmake") || match(ext, ".txt")) {
        const char* name = path;
        for (const char* p = path; *p; ++p) {
            if (*p == '/' || *p == '\\') {
                name = p + 1;
            }
        }
        if (match(name, "CMakeLists.txt")) {
            return DocumentType::CMake;
        }
        return DocumentType::PlainText;
    }
    if (match(ext, ".glsl") || match(ext, ".vert") || match(ext, ".frag") || match(ext, ".comp")) {
        return DocumentType::Glsl;
    }

    return DocumentType::Unknown;
}

}
