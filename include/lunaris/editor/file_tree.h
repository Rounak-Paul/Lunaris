#pragma once

#include <cstdint>

namespace lunaris {

class Theme;
class FileOperations;

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
    void set_file_operations(FileOperations* ops) { _file_ops = ops; }

    bool open_folder(const char* path);
    void close_folder();
    bool has_folder() const { return _root != nullptr; }
    void refresh();

    const char* get_root_path() const;
    const char* get_root_name() const;

    void on_ui();
    void draw_toolbar();

    using FileSelectedCallback = void(*)(const char* path, void* user_data);
    void set_file_selected_callback(FileSelectedCallback cb, void* user_data) {
        _on_file_selected = cb;
        _callback_user_data = user_data;
    }

private:
    void load_children(FileNode* node);
    void draw_node(FileNode* node, int depth, uint32_t continuation_mask);
    void draw_context_menu();
    void sort_children(FileNode* node);
    void collapse_all(FileNode* node);
    FileNode* find_node(const char* path);
    FileNode* find_node_recursive(FileNode* node, const char* path);

    Theme* _theme;
    FileOperations* _file_ops;
    FileNode* _root;
    FileNode* _context_node;
    FileNode* _selected_node;
    bool _show_context_menu;
    bool _show_rename_popup;
    bool _show_new_file_popup;
    bool _show_new_folder_popup;
    char _rename_buffer[256];
    char _new_item_buffer[256];
    FileSelectedCallback _on_file_selected;
    void* _callback_user_data;
};

}
