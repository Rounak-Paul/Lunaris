#include "lunaris/editor/file_operations.h"
#include "lunaris/editor/undo_manager.h"
#include "lunaris/editor/document_manager.h"
#include "lunaris/editor/sidebar.h"
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

namespace lunaris {

FileOperations::FileOperations()
    : _doc_manager(nullptr)
    , _sidebar(nullptr) {
}

FileOperations::~FileOperations() {
}

bool FileOperations::file_exists(const char* path) const {
    struct stat st;
    if (stat(path, &st) != 0) return false;
    return S_ISREG(st.st_mode);
}

bool FileOperations::folder_exists(const char* path) const {
    struct stat st;
    if (stat(path, &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}

bool FileOperations::read_file_content(const char* path, char** out_content, size_t* out_len) const {
    FILE* f = fopen(path, "rb");
    if (!f) {
        *out_content = nullptr;
        *out_len = 0;
        return false;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size < 0 || static_cast<size_t>(size) > MAX_CONTENT_LEN) {
        fclose(f);
        *out_content = nullptr;
        *out_len = 0;
        return false;
    }

    char* content = new char[size + 1];
    size_t read = fread(content, 1, size, f);
    content[read] = '\0';
    fclose(f);

    *out_content = content;
    *out_len = read;
    return true;
}

bool FileOperations::write_file(const char* path, const char* content, size_t len) {
    FILE* f = fopen(path, "wb");
    if (!f) return false;
    size_t written = fwrite(content, 1, len, f);
    fclose(f);
    return written == len;
}

bool FileOperations::remove_file(const char* path) {
    return unlink(path) == 0;
}

bool FileOperations::make_directory(const char* path) {
    return mkdir(path, 0755) == 0;
}

bool FileOperations::remove_directory(const char* path) {
    return rmdir(path) == 0;
}

bool FileOperations::rename_path(const char* old_path, const char* new_path) {
    return rename(old_path, new_path) == 0;
}

void FileOperations::get_duplicate_path(const char* path, char* out_path, size_t out_size) const {
    size_t len = strlen(path);
    const char* dot = nullptr;
    for (size_t i = len; i > 0; --i) {
        if (path[i - 1] == '.') {
            dot = path + i - 1;
            break;
        }
        if (path[i - 1] == '/' || path[i - 1] == '\\') break;
    }

    if (dot) {
        size_t base_len = dot - path;
        int num = 1;
        while (true) {
            snprintf(out_path, out_size, "%.*s_copy%d%s", (int)base_len, path, num, dot);
            if (!file_exists(out_path)) break;
            ++num;
        }
    } else {
        int num = 1;
        while (true) {
            snprintf(out_path, out_size, "%s_copy%d", path, num);
            if (!file_exists(out_path)) break;
            ++num;
        }
    }
}

bool FileOperations::create_file(const char* path) {
    if (file_exists(path)) return false;

    FILE* f = fopen(path, "wb");
    if (!f) return false;
    fclose(f);

    UndoManager::instance().record_file_create(path);

    if (_sidebar) {
        _sidebar->refresh_file_tree();
    }

    return true;
}

bool FileOperations::delete_file(const char* path) {
    if (!file_exists(path)) return false;

    char* content = nullptr;
    size_t content_len = 0;
    read_file_content(path, &content, &content_len);

    if (_doc_manager) {
        Document* doc = _doc_manager->find_by_path(path);
        if (doc) {
            _doc_manager->close_document(doc->get_id());
        }
    }

    if (!remove_file(path)) {
        delete[] content;
        return false;
    }

    UndoManager::instance().record_file_delete(path, content, content_len);
    delete[] content;

    if (_sidebar) {
        _sidebar->refresh_file_tree();
    }

    return true;
}

bool FileOperations::rename_file(const char* old_path, const char* new_path) {
    if (!file_exists(old_path)) return false;
    if (file_exists(new_path)) return false;

    if (!rename_path(old_path, new_path)) return false;

    UndoManager::instance().record_file_rename(old_path, new_path);

    if (_sidebar) {
        _sidebar->refresh_file_tree();
    }

    return true;
}

bool FileOperations::duplicate_file(const char* path) {
    if (!file_exists(path)) return false;

    char new_path[MAX_PATH_LEN];
    get_duplicate_path(path, new_path, sizeof(new_path));

    char* content = nullptr;
    size_t content_len = 0;
    if (!read_file_content(path, &content, &content_len)) {
        return false;
    }

    bool ok = write_file(new_path, content, content_len);
    delete[] content;

    if (!ok) return false;

    UndoManager::instance().record_file_create(new_path);

    if (_sidebar) {
        _sidebar->refresh_file_tree();
    }

    return true;
}

bool FileOperations::create_folder(const char* path) {
    if (folder_exists(path)) return false;

    if (!make_directory(path)) return false;

    UndoManager::instance().record_folder_create(path);

    if (_sidebar) {
        _sidebar->refresh_file_tree();
    }

    return true;
}

bool FileOperations::delete_folder(const char* path) {
    if (!folder_exists(path)) return false;

    if (!remove_directory(path)) return false;

    UndoManager::instance().record_folder_delete(path);

    if (_sidebar) {
        _sidebar->refresh_file_tree();
    }

    return true;
}

bool FileOperations::rename_folder(const char* old_path, const char* new_path) {
    if (!folder_exists(old_path)) return false;
    if (folder_exists(new_path)) return false;

    if (!rename_path(old_path, new_path)) return false;

    UndoManager::instance().record_folder_rename(old_path, new_path);

    if (_sidebar) {
        _sidebar->refresh_file_tree();
    }

    return true;
}

bool FileOperations::apply_undo(const UndoAction* action) {
    if (!action) return false;

    switch (action->type) {
        case UndoActionType::FileCreate:
            if (file_exists(action->path)) {
                remove_file(action->path);
                if (_sidebar) _sidebar->refresh_file_tree();
                return true;
            }
            break;

        case UndoActionType::FileDelete:
            if (!file_exists(action->path)) {
                if (action->content && action->content_len > 0) {
                    write_file(action->path, action->content, action->content_len);
                } else {
                    FILE* f = fopen(action->path, "wb");
                    if (f) fclose(f);
                }
                if (_sidebar) _sidebar->refresh_file_tree();
                return true;
            }
            break;

        case UndoActionType::FileRename:
            if (file_exists(action->path_alt)) {
                rename_path(action->path_alt, action->path);
                if (_sidebar) _sidebar->refresh_file_tree();
                return true;
            }
            break;

        case UndoActionType::FolderCreate:
            if (folder_exists(action->path)) {
                remove_directory(action->path);
                if (_sidebar) _sidebar->refresh_file_tree();
                return true;
            }
            break;

        case UndoActionType::FolderDelete:
            if (!folder_exists(action->path)) {
                make_directory(action->path);
                if (_sidebar) _sidebar->refresh_file_tree();
                return true;
            }
            break;

        case UndoActionType::FolderRename:
            if (folder_exists(action->path_alt)) {
                rename_path(action->path_alt, action->path);
                if (_sidebar) _sidebar->refresh_file_tree();
                return true;
            }
            break;

        default:
            break;
    }
    return false;
}

bool FileOperations::apply_redo(const UndoAction* action) {
    if (!action) return false;

    switch (action->type) {
        case UndoActionType::FileCreate:
            if (!file_exists(action->path)) {
                FILE* f = fopen(action->path, "wb");
                if (f) fclose(f);
                if (_sidebar) _sidebar->refresh_file_tree();
                return true;
            }
            break;

        case UndoActionType::FileDelete:
            if (file_exists(action->path)) {
                if (_doc_manager) {
                    Document* doc = _doc_manager->find_by_path(action->path);
                    if (doc) {
                        _doc_manager->close_document(doc->get_id());
                    }
                }
                remove_file(action->path);
                if (_sidebar) _sidebar->refresh_file_tree();
                return true;
            }
            break;

        case UndoActionType::FileRename:
            if (file_exists(action->path)) {
                rename_path(action->path, action->path_alt);
                if (_sidebar) _sidebar->refresh_file_tree();
                return true;
            }
            break;

        case UndoActionType::FolderCreate:
            if (!folder_exists(action->path)) {
                make_directory(action->path);
                if (_sidebar) _sidebar->refresh_file_tree();
                return true;
            }
            break;

        case UndoActionType::FolderDelete:
            if (folder_exists(action->path)) {
                remove_directory(action->path);
                if (_sidebar) _sidebar->refresh_file_tree();
                return true;
            }
            break;

        case UndoActionType::FolderRename:
            if (folder_exists(action->path)) {
                rename_path(action->path, action->path_alt);
                if (_sidebar) _sidebar->refresh_file_tree();
                return true;
            }
            break;

        default:
            break;
    }
    return false;
}

}
