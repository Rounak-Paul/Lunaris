#include "lunaris/editor/undo_manager.h"
#include <cstring>

namespace lunaris {

UndoManager& UndoManager::instance() {
    static UndoManager instance;
    return instance;
}

UndoManager::UndoManager()
    : _undo_stack(nullptr)
    , _undo_count(0)
    , _redo_stack(nullptr)
    , _redo_count(0) {
    _undo_stack = new UndoAction[MAX_HISTORY];
    _redo_stack = new UndoAction[MAX_HISTORY];
    for (size_t i = 0; i < MAX_HISTORY; ++i) {
        _undo_stack[i] = {};
        _redo_stack[i] = {};
    }
}

UndoManager::~UndoManager() {
    clear();
    delete[] _undo_stack;
    delete[] _redo_stack;
}

void UndoManager::record_text_insert(DocumentID doc_id, const char* filepath, size_t pos, const char* text, size_t len, size_t cursor_before, size_t cursor_after) {
    UndoAction action = {};
    action.type = UndoActionType::TextInsert;
    action.doc_id = doc_id;
    action.pos = pos;
    action.text = new char[len + 1];
    memcpy(action.text, text, len);
    action.text[len] = '\0';
    action.len = len;
    action.cursor_before = cursor_before;
    action.cursor_after = cursor_after;
    if (filepath && filepath[0] != '\0') {
        size_t path_len = strlen(filepath);
        action.path = new char[path_len + 1];
        memcpy(action.path, filepath, path_len + 1);
    }
    push_undo(action);
    clear_redo();
}

void UndoManager::record_text_delete(DocumentID doc_id, const char* filepath, size_t pos, const char* text, size_t len, size_t cursor_before, size_t cursor_after) {
    UndoAction action = {};
    action.type = UndoActionType::TextDelete;
    action.doc_id = doc_id;
    action.pos = pos;
    action.text = new char[len + 1];
    memcpy(action.text, text, len);
    action.text[len] = '\0';
    action.len = len;
    action.cursor_before = cursor_before;
    action.cursor_after = cursor_after;
    if (filepath && filepath[0] != '\0') {
        size_t path_len = strlen(filepath);
        action.path = new char[path_len + 1];
        memcpy(action.path, filepath, path_len + 1);
    }
    push_undo(action);
    clear_redo();
}

void UndoManager::record_file_create(const char* path) {
    UndoAction action = {};
    action.type = UndoActionType::FileCreate;
    size_t path_len = strlen(path);
    action.path = new char[path_len + 1];
    memcpy(action.path, path, path_len + 1);
    push_undo(action);
    clear_redo();
}

void UndoManager::record_file_delete(const char* path, const char* content, size_t content_len) {
    UndoAction action = {};
    action.type = UndoActionType::FileDelete;
    size_t path_len = strlen(path);
    action.path = new char[path_len + 1];
    memcpy(action.path, path, path_len + 1);
    if (content && content_len > 0) {
        action.content = new char[content_len + 1];
        memcpy(action.content, content, content_len);
        action.content[content_len] = '\0';
        action.content_len = content_len;
    }
    push_undo(action);
    clear_redo();
}

void UndoManager::record_file_rename(const char* old_path, const char* new_path) {
    UndoAction action = {};
    action.type = UndoActionType::FileRename;
    size_t old_len = strlen(old_path);
    size_t new_len = strlen(new_path);
    action.path = new char[old_len + 1];
    action.path_alt = new char[new_len + 1];
    memcpy(action.path, old_path, old_len + 1);
    memcpy(action.path_alt, new_path, new_len + 1);
    push_undo(action);
    clear_redo();
}

void UndoManager::record_folder_create(const char* path) {
    UndoAction action = {};
    action.type = UndoActionType::FolderCreate;
    size_t path_len = strlen(path);
    action.path = new char[path_len + 1];
    memcpy(action.path, path, path_len + 1);
    push_undo(action);
    clear_redo();
}

void UndoManager::record_folder_delete(const char* path) {
    UndoAction action = {};
    action.type = UndoActionType::FolderDelete;
    size_t path_len = strlen(path);
    action.path = new char[path_len + 1];
    memcpy(action.path, path, path_len + 1);
    push_undo(action);
    clear_redo();
}

void UndoManager::record_folder_rename(const char* old_path, const char* new_path) {
    UndoAction action = {};
    action.type = UndoActionType::FolderRename;
    size_t old_len = strlen(old_path);
    size_t new_len = strlen(new_path);
    action.path = new char[old_len + 1];
    action.path_alt = new char[new_len + 1];
    memcpy(action.path, old_path, old_len + 1);
    memcpy(action.path_alt, new_path, new_len + 1);
    push_undo(action);
    clear_redo();
}

bool UndoManager::is_text_action(UndoActionType type) const {
    return type == UndoActionType::TextInsert || type == UndoActionType::TextDelete;
}

bool UndoManager::is_file_action(UndoActionType type) const {
    return !is_text_action(type);
}

void UndoManager::push_undo(UndoAction action) {
    if (_undo_count >= MAX_HISTORY) {
        free_action(_undo_stack[0]);
        for (size_t i = 0; i < MAX_HISTORY - 1; ++i) {
            _undo_stack[i] = _undo_stack[i + 1];
        }
        --_undo_count;
    }
    _undo_stack[_undo_count] = action;
    ++_undo_count;
}

void UndoManager::push_redo(UndoAction action) {
    if (_redo_count >= MAX_HISTORY) {
        free_action(_redo_stack[0]);
        for (size_t i = 0; i < MAX_HISTORY - 1; ++i) {
            _redo_stack[i] = _redo_stack[i + 1];
        }
        --_redo_count;
    }
    _redo_stack[_redo_count] = action;
    ++_redo_count;
}

const UndoAction* UndoManager::peek_undo() const {
    if (_undo_count == 0) return nullptr;
    return &_undo_stack[_undo_count - 1];
}

const UndoAction* UndoManager::peek_redo() const {
    if (_redo_count == 0) return nullptr;
    return &_redo_stack[_redo_count - 1];
}

UndoAction UndoManager::copy_action(const UndoAction& action) {
    UndoAction copy = {};
    copy.type = action.type;
    copy.doc_id = action.doc_id;
    copy.pos = action.pos;
    copy.len = action.len;
    copy.cursor_before = action.cursor_before;
    copy.cursor_after = action.cursor_after;
    copy.content_len = action.content_len;
    
    if (action.text) {
        copy.text = new char[action.len + 1];
        memcpy(copy.text, action.text, action.len + 1);
    }
    if (action.path) {
        size_t len = strlen(action.path);
        copy.path = new char[len + 1];
        memcpy(copy.path, action.path, len + 1);
    }
    if (action.path_alt) {
        size_t len = strlen(action.path_alt);
        copy.path_alt = new char[len + 1];
        memcpy(copy.path_alt, action.path_alt, len + 1);
    }
    if (action.content) {
        copy.content = new char[action.content_len + 1];
        memcpy(copy.content, action.content, action.content_len + 1);
    }
    return copy;
}

UndoAction* UndoManager::undo() {
    if (_undo_count == 0) return nullptr;

    --_undo_count;
    UndoAction& action = _undo_stack[_undo_count];
    UndoAction redo_action = copy_action(action);
    push_redo(redo_action);
    return &_redo_stack[_redo_count - 1];
}

UndoAction* UndoManager::redo() {
    if (_redo_count == 0) return nullptr;

    --_redo_count;
    UndoAction& action = _redo_stack[_redo_count];
    UndoAction undo_action = copy_action(action);
    _undo_stack[_undo_count] = undo_action;
    ++_undo_count;
    return &_undo_stack[_undo_count - 1];
}

void UndoManager::clear_redo() {
    for (size_t i = 0; i < _redo_count; ++i) {
        free_action(_redo_stack[i]);
    }
    _redo_count = 0;
}

void UndoManager::free_action(UndoAction& action) {
    delete[] action.text;
    delete[] action.path;
    delete[] action.path_alt;
    delete[] action.content;
    action.text = nullptr;
    action.path = nullptr;
    action.path_alt = nullptr;
    action.content = nullptr;
}

void UndoManager::clear() {
    for (size_t i = 0; i < _undo_count; ++i) {
        free_action(_undo_stack[i]);
    }
    _undo_count = 0;
    clear_redo();
}

void UndoManager::clear_for_document(DocumentID doc_id) {
    size_t write = 0;
    for (size_t i = 0; i < _undo_count; ++i) {
        if (is_text_action(_undo_stack[i].type) && _undo_stack[i].doc_id == doc_id) {
            free_action(_undo_stack[i]);
        } else {
            if (write != i) {
                _undo_stack[write] = _undo_stack[i];
            }
            ++write;
        }
    }
    _undo_count = write;

    write = 0;
    for (size_t i = 0; i < _redo_count; ++i) {
        if (is_text_action(_redo_stack[i].type) && _redo_stack[i].doc_id == doc_id) {
            free_action(_redo_stack[i]);
        } else {
            if (write != i) {
                _redo_stack[write] = _redo_stack[i];
            }
            ++write;
        }
    }
    _redo_count = write;
}

}
