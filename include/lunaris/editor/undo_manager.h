#pragma once

#include <cstdint>
#include <cstddef>

namespace lunaris {

using DocumentID = uint32_t;

enum class UndoActionType : uint8_t {
    TextInsert,
    TextDelete,
    FileCreate,
    FileDelete,
    FileRename,
    FolderCreate,
    FolderDelete,
    FolderRename
};

struct UndoAction {
    UndoActionType type;
    DocumentID doc_id;
    size_t pos;
    char* text;
    size_t len;
    size_t cursor_before;
    size_t cursor_after;
    char* path;
    char* path_alt;
    char* content;
    size_t content_len;
};

class UndoManager {
public:
    static constexpr size_t MAX_HISTORY = 1000;
    static constexpr size_t MAX_PATH_LEN = 1024;

    static UndoManager& instance();

    void record_text_insert(DocumentID doc_id, const char* filepath, size_t pos, const char* text, size_t len, size_t cursor_before, size_t cursor_after);
    void record_text_delete(DocumentID doc_id, const char* filepath, size_t pos, const char* text, size_t len, size_t cursor_before, size_t cursor_after);

    void record_file_create(const char* path);
    void record_file_delete(const char* path, const char* content, size_t content_len);
    void record_file_rename(const char* old_path, const char* new_path);

    void record_folder_create(const char* path);
    void record_folder_delete(const char* path);
    void record_folder_rename(const char* old_path, const char* new_path);

    bool can_undo() const { return _undo_count > 0; }
    bool can_redo() const { return _redo_count > 0; }

    const UndoAction* peek_undo() const;
    const UndoAction* peek_redo() const;

    UndoAction* undo();
    UndoAction* redo();

    void clear();
    void clear_for_document(DocumentID doc_id);

    bool is_text_action(UndoActionType type) const;
    bool is_file_action(UndoActionType type) const;

private:
    UndoManager();
    ~UndoManager();

    void push_undo(UndoAction action);
    void push_redo(UndoAction action);
    void clear_redo();
    void free_action(UndoAction& action);
    UndoAction copy_action(const UndoAction& action);

    UndoAction* _undo_stack;
    size_t _undo_count;
    UndoAction* _redo_stack;
    size_t _redo_count;
};

}
