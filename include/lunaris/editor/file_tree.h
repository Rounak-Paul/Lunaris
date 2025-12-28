#pragma once

#include <cstdint>

namespace lunaris {

class Theme;

struct FileNode {
    static constexpr uint32_t MAX_NAME_LENGTH = 256;
    static constexpr uint32_t MAX_PATH_LENGTH = 1024;
    static constexpr uint32_t MAX_CHILDREN = 512;

    char name[MAX_NAME_LENGTH];
    char path[MAX_PATH_LENGTH];
    bool is_directory;
    bool is_expanded;
    bool is_loaded;
    FileNode* children;
    uint32_t child_count;
    FileNode* parent;

    FileNode();
    ~FileNode();

    void clear_children();
};

class FileTree {
public:
    FileTree();
    ~FileTree();

    void set_theme(Theme* theme) { _theme = theme; }

    bool open_folder(const char* path);
    void close_folder();
    bool has_folder() const { return _root != nullptr; }

    const char* get_root_path() const;
    const char* get_root_name() const;

    void on_ui();

    using FileSelectedCallback = void(*)(const char* path, void* user_data);
    void set_file_selected_callback(FileSelectedCallback cb, void* user_data) {
        _on_file_selected = cb;
        _callback_user_data = user_data;
    }

private:
    void load_children(FileNode* node);
    void draw_node(FileNode* node, int depth, uint32_t continuation_mask);
    void sort_children(FileNode* node);

    Theme* _theme;
    FileNode* _root;
    FileSelectedCallback _on_file_selected;
    void* _callback_user_data;
};

}
