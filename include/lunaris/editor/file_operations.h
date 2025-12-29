#pragma once

#include <cstdint>
#include <cstddef>

namespace lunaris {

class DocumentManager;
class Sidebar;

class FileOperations {
public:
    static constexpr size_t MAX_PATH_LEN = 1024;
    static constexpr size_t MAX_CONTENT_LEN = 16 * 1024 * 1024;

    FileOperations();
    ~FileOperations();

    void set_document_manager(DocumentManager* mgr) { _doc_manager = mgr; }
    void set_sidebar(Sidebar* sb) { _sidebar = sb; }

    bool create_file(const char* path);
    bool delete_file(const char* path);
    bool rename_file(const char* old_path, const char* new_path);
    bool duplicate_file(const char* path);

    bool create_folder(const char* path);
    bool delete_folder(const char* path);
    bool rename_folder(const char* old_path, const char* new_path);

    bool apply_undo(const struct UndoAction* action);
    bool apply_redo(const struct UndoAction* action);

    bool file_exists(const char* path) const;
    bool folder_exists(const char* path) const;
    bool read_file_content(const char* path, char** out_content, size_t* out_len) const;

private:
    bool write_file(const char* path, const char* content, size_t len);
    bool remove_file(const char* path);
    bool make_directory(const char* path);
    bool remove_directory(const char* path);
    bool rename_path(const char* old_path, const char* new_path);
    void get_duplicate_path(const char* path, char* out_path, size_t out_size) const;

    DocumentManager* _doc_manager;
    Sidebar* _sidebar;
};

}
