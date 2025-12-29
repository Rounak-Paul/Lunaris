#include "lunaris/editor/file_tree.h"
#include "lunaris/editor/file_operations.h"
#include "lunaris/core/theme.h"
#include <imgui.h>
#include <tinyvk/core/file_dialog.h>
#include <tinyvk/assets/icons_font_awesome.h>
#include <cstring>
#include <algorithm>
#include <filesystem>

namespace lunaris {

FileNode::FileNode()
    : is_directory(false)
    , is_expanded(false)
    , is_loaded(false)
    , children(nullptr)
    , child_count(0)
    , parent(nullptr) {
    name[0] = '\0';
    path[0] = '\0';
}

FileNode::~FileNode() {
    clear_children();
}

void FileNode::clear_children() {
    if (children) {
        delete[] children;
        children = nullptr;
    }
    child_count = 0;
    is_loaded = false;
}

FileTree::FileTree()
    : _theme(nullptr)
    , _file_ops(nullptr)
    , _root(nullptr)
    , _context_node(nullptr)
    , _selected_node(nullptr)
    , _show_context_menu(false)
    , _show_rename_popup(false)
    , _show_new_file_popup(false)
    , _show_new_folder_popup(false)
    , _on_file_selected(nullptr)
    , _callback_user_data(nullptr) {
    _rename_buffer[0] = '\0';
    _new_item_buffer[0] = '\0';
}

FileTree::~FileTree() {
    close_folder();
}

bool FileTree::open_folder(const char* path) {
    if (!path || path[0] == '\0') {
        return false;
    }

    close_folder();

    _root = new FileNode();
    strncpy(_root->path, path, FileNode::MAX_PATH_LENGTH - 1);
    _root->path[FileNode::MAX_PATH_LENGTH - 1] = '\0';

    const char* last_sep = strrchr(path, '/');
    if (!last_sep) {
        last_sep = strrchr(path, '\\');
    }
    const char* folder_name = last_sep ? last_sep + 1 : path;
    strncpy(_root->name, folder_name, FileNode::MAX_NAME_LENGTH - 1);
    _root->name[FileNode::MAX_NAME_LENGTH - 1] = '\0';

    _root->is_directory = true;
    _root->is_expanded = true;

    load_children(_root);

    return true;
}

void FileTree::close_folder() {
    if (_root) {
        delete _root;
        _root = nullptr;
    }
}

const char* FileTree::get_root_path() const {
    return _root ? _root->path : nullptr;
}

const char* FileTree::get_root_name() const {
    return _root ? _root->name : nullptr;
}

void FileTree::load_children(FileNode* node) {
    if (!node || !node->is_directory || node->is_loaded) {
        return;
    }

    node->clear_children();

    std::vector<std::filesystem::directory_entry> entries;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(node->path)) {
            std::string filename = entry.path().filename().string();
            if (!filename.empty() && filename[0] != '.') {
                entries.push_back(entry);
            }
        }
    } catch (...) {
        node->is_loaded = true;
        return;
    }

    if (entries.empty()) {
        node->is_loaded = true;
        return;
    }

    uint32_t count = static_cast<uint32_t>(entries.size());
    if (count > FileNode::MAX_CHILDREN) {
        count = FileNode::MAX_CHILDREN;
    }

    node->children = new FileNode[count];
    node->child_count = count;

    for (uint32_t i = 0; i < count; ++i) {
        const auto& entry = entries[i];
        FileNode& child = node->children[i];

        std::string name_str = entry.path().filename().string();
        std::string path_str = entry.path().string();

        strncpy(child.name, name_str.c_str(), FileNode::MAX_NAME_LENGTH - 1);
        child.name[FileNode::MAX_NAME_LENGTH - 1] = '\0';

        strncpy(child.path, path_str.c_str(), FileNode::MAX_PATH_LENGTH - 1);
        child.path[FileNode::MAX_PATH_LENGTH - 1] = '\0';

        child.is_directory = entry.is_directory();
        child.parent = node;
    }

    sort_children(node);
    node->is_loaded = true;
}

void FileTree::sort_children(FileNode* node) {
    if (!node || !node->children || node->child_count < 2) {
        return;
    }

    std::sort(node->children, node->children + node->child_count,
        [](const FileNode& a, const FileNode& b) {
            if (a.is_directory != b.is_directory) {
                return a.is_directory > b.is_directory;
            }
            return strcasecmp(a.name, b.name) < 0;
        });
}

void FileTree::refresh() {
    if (!_root) return;
    char path[FileNode::MAX_PATH_LENGTH];
    strncpy(path, _root->path, FileNode::MAX_PATH_LENGTH);
    open_folder(path);
}

void FileTree::collapse_all(FileNode* node) {
    if (!node) return;
    if (node != _root) {
        node->is_expanded = false;
    }
    for (uint32_t i = 0; i < node->child_count; ++i) {
        collapse_all(&node->children[i]);
    }
}

FileNode* FileTree::find_node(const char* path) {
    return find_node_recursive(_root, path);
}

FileNode* FileTree::find_node_recursive(FileNode* node, const char* path) {
    if (!node) return nullptr;
    if (strcmp(node->path, path) == 0) return node;
    for (uint32_t i = 0; i < node->child_count; ++i) {
        FileNode* found = find_node_recursive(&node->children[i], path);
        if (found) return found;
    }
    return nullptr;
}

void FileTree::draw_toolbar() {
    if (!_root) return;

    Color text_dim = _theme ? _theme->get_text_dim() : Color(0.5f, 0.5f, 0.52f);
    Color accent = _theme ? _theme->get_accent() : Color(0.3f, 0.5f, 0.8f);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(accent.r, accent.g, accent.b, 0.2f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(accent.r, accent.g, accent.b, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(text_dim.r, text_dim.g, text_dim.b, 1.0f));

    float icon_size = 18.0f;

    if (ImGui::Button(ICON_FA_FILE_CIRCLE_PLUS, ImVec2(icon_size, icon_size))) {
        _context_node = _root;
        _new_item_buffer[0] = '\0';
        _show_new_file_popup = true;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("New File");

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FOLDER_PLUS, ImVec2(icon_size, icon_size))) {
        _context_node = _root;
        _new_item_buffer[0] = '\0';
        _show_new_folder_popup = true;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("New Folder");

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROWS_ROTATE, ImVec2(icon_size, icon_size))) {
        refresh();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Refresh");

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_COMPRESS, ImVec2(icon_size, icon_size))) {
        collapse_all(_root);
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Collapse All");

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
}

void FileTree::on_ui() {
    if (!_root) {
        return;
    }

    draw_node(_root, 0, 0);
    draw_context_menu();
}

void FileTree::draw_node(FileNode* node, int depth, uint32_t continuation_mask) {
    if (!node) {
        return;
    }

    Color text = _theme ? _theme->get_text() : Color(0.9f, 0.9f, 0.92f);
    Color text_dim = _theme ? _theme->get_text_dim() : Color(0.5f, 0.5f, 0.52f);
    Color accent = _theme ? _theme->get_accent() : Color(0.3f, 0.5f, 0.8f);
    Color line_col = _theme ? _theme->get_border() : Color(0.3f, 0.3f, 0.32f);

    bool is_root = (node == _root);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    float line_height = ImGui::GetTextLineHeightWithSpacing();
    float indent_width = 16.0f;
    float half_height = line_height * 0.5f;

    ImU32 line_color = IM_COL32(
        (int)(line_col.r * 255),
        (int)(line_col.g * 255),
        (int)(line_col.b * 255),
        100
    );

    if (!is_root) {
        for (int i = 1; i < depth; ++i) {
            if (continuation_mask & (1u << i)) {
                float x = cursor.x + (i - 1) * indent_width + indent_width * 0.5f;
                draw_list->AddLine(
                    ImVec2(x, cursor.y),
                    ImVec2(x, cursor.y + line_height),
                    line_color, 1.0f
                );
            }
        }

        if (depth > 0) {
            float x = cursor.x + (depth - 1) * indent_width + indent_width * 0.5f;
            bool has_more_siblings = (continuation_mask & (1u << depth)) != 0;

            draw_list->AddLine(
                ImVec2(x, cursor.y),
                ImVec2(x, cursor.y + half_height),
                line_color, 1.0f
            );
            draw_list->AddLine(
                ImVec2(x, cursor.y + half_height),
                ImVec2(x + indent_width * 0.4f, cursor.y + half_height),
                line_color, 1.0f
            );

            if (has_more_siblings) {
                draw_list->AddLine(
                    ImVec2(x, cursor.y + half_height),
                    ImVec2(x, cursor.y + line_height),
                    line_color, 1.0f
                );
            }
        }
    }

    float content_x = is_root ? 0.0f : (depth * indent_width);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + content_x);

    ImGui::PushID(node->path);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(accent.r, accent.g, accent.b, 0.15f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(accent.r, accent.g, accent.b, 0.2f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(accent.r, accent.g, accent.b, 0.3f));

    bool is_open = node->is_expanded;

    if (node->is_directory) {
        float circle_radius = 4.0f;
        ImVec2 circle_pos = ImGui::GetCursorScreenPos();
        circle_pos.x += circle_radius + 1.0f;
        circle_pos.y += line_height * 0.5f;

        ImU32 circle_color = IM_COL32(
            (int)(text_dim.r * 255),
            (int)(text_dim.g * 255),
            (int)(text_dim.b * 255),
            200
        );

        ImGui::InvisibleButton("##toggle", ImVec2(circle_radius * 2.0f + 4.0f, line_height));
        if (ImGui::IsItemClicked()) {
            is_open = !is_open;
            node->is_expanded = is_open;
        }

        if (is_open) {
            draw_list->AddCircleFilled(circle_pos, circle_radius, circle_color);
        } else {
            draw_list->AddCircle(circle_pos, circle_radius, circle_color, 0, 1.5f);
        }

        ImGui::SameLine(0.0f, 4.0f);
    } else {
        ImGui::Dummy(ImVec2(12.0f, 0.0f));
        ImGui::SameLine(0.0f, 4.0f);
    }

    const char* icon;
    ImVec4 icon_col;

    if (node->is_directory) {
        icon = is_open ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER;
        icon_col = ImVec4(0.9f, 0.75f, 0.4f, 1.0f);
    } else {
        icon = ICON_FA_FILE;
        icon_col = ImVec4(text_dim.r, text_dim.g, text_dim.b, 0.7f);
    }

    ImVec2 row_start = ImGui::GetCursorScreenPos();

    float font_size = ImGui::GetFontSize();
    float icon_scale = 0.8f;
    float scaled_size = font_size * icon_scale;
    float y_offset = (font_size - scaled_size) * 0.5f;

    ImVec2 icon_pos = ImGui::GetCursorPos();
    ImGui::SetCursorPosY(icon_pos.y + y_offset);
    ImGui::SetWindowFontScale(icon_scale);
    ImGui::TextColored(icon_col, "%s", icon);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::SameLine(0.0f, 4.0f);
    ImGui::SetCursorPosY(icon_pos.y);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(text.r, text.g, text.b, 1.0f));

    bool is_selected = (_selected_node == node);
    if (is_selected) {
        ImVec2 sel_start = row_start;
        sel_start.x -= 4.0f;
        ImVec2 sel_end = sel_start;
        sel_end.x += ImGui::GetContentRegionAvail().x + 50.0f;
        sel_end.y += line_height;
        draw_list->AddRectFilled(sel_start, sel_end, IM_COL32((int)(accent.r * 255), (int)(accent.g * 255), (int)(accent.b * 255), 40));
    }

    ImGui::TextUnformatted(node->name);

    ImVec2 row_end = ImGui::GetCursorScreenPos();
    row_end.x = row_start.x + ImGui::GetContentRegionAvail().x + 50.0f;
    row_end.y = row_start.y + line_height;

    bool row_hovered = ImGui::IsMouseHoveringRect(row_start, row_end);
    bool row_left_clicked = row_hovered && ImGui::IsMouseClicked(0);
    bool row_right_clicked = row_hovered && ImGui::IsMouseClicked(1);

    if (row_left_clicked) {
        _selected_node = node;
        if (node->is_directory) {
            is_open = !is_open;
            node->is_expanded = is_open;
        } else if (_on_file_selected) {
            _on_file_selected(node->path, _callback_user_data);
        }
    }

    if (row_right_clicked) {
        _context_node = node;
        _selected_node = node;
        _show_context_menu = true;
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
    ImGui::PopID();

    if (node->is_directory && is_open) {
        if (!node->is_loaded) {
            load_children(node);
        }

        for (uint32_t i = 0; i < node->child_count; ++i) {
            bool is_last = (i == node->child_count - 1);
            uint32_t child_mask = continuation_mask;
            if (!is_last) {
                child_mask |= (1u << (depth + 1));
            }
            draw_node(&node->children[i], depth + 1, child_mask);
        }
    }
}

void FileTree::draw_context_menu() {
    if (_show_context_menu) {
        ImGui::OpenPopup("##FileTreeContext");
        _show_context_menu = false;
    }

    if (ImGui::BeginPopup("##FileTreeContext")) {
        FileNode* target = _context_node ? _context_node : _root;
        FileNode* parent_folder = target->is_directory ? target : target->parent;

        if (ImGui::MenuItem(ICON_FA_FILE_CIRCLE_PLUS "  New File")) {
            _context_node = parent_folder;
            _new_item_buffer[0] = '\0';
            _show_new_file_popup = true;
        }
        if (ImGui::MenuItem(ICON_FA_FOLDER_PLUS "  New Folder")) {
            _context_node = parent_folder;
            _new_item_buffer[0] = '\0';
            _show_new_folder_popup = true;
        }

        ImGui::Separator();

        if (target != _root) {
            if (ImGui::MenuItem(ICON_FA_PEN "  Rename")) {
                strncpy(_rename_buffer, target->name, sizeof(_rename_buffer) - 1);
                _show_rename_popup = true;
            }
            if (!target->is_directory && ImGui::MenuItem(ICON_FA_COPY "  Duplicate")) {
                if (_file_ops) {
                    _file_ops->duplicate_file(target->path);
                }
            }
            if (ImGui::MenuItem(ICON_FA_TRASH "  Delete")) {
                if (_file_ops) {
                    if (target->is_directory) {
                        _file_ops->delete_folder(target->path);
                    } else {
                        _file_ops->delete_file(target->path);
                    }
                }
            }
        }

        ImGui::EndPopup();
    }

    if (_show_new_file_popup) {
        ImGui::OpenPopup("New File##popup");
        _show_new_file_popup = false;
    }

    if (ImGui::BeginPopupModal("New File##popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter file name:");
        ImGui::SetKeyboardFocusHere();
        bool enter = ImGui::InputText("##newfile", _new_item_buffer, sizeof(_new_item_buffer), ImGuiInputTextFlags_EnterReturnsTrue);

        if (ImGui::Button("Create") || enter) {
            if (_new_item_buffer[0] != '\0' && _file_ops && _context_node) {
                char path[FileNode::MAX_PATH_LENGTH];
                snprintf(path, sizeof(path), "%s/%s", _context_node->path, _new_item_buffer);
                _file_ops->create_file(path);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (_show_new_folder_popup) {
        ImGui::OpenPopup("New Folder##popup");
        _show_new_folder_popup = false;
    }

    if (ImGui::BeginPopupModal("New Folder##popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter folder name:");
        ImGui::SetKeyboardFocusHere();
        bool enter = ImGui::InputText("##newfolder", _new_item_buffer, sizeof(_new_item_buffer), ImGuiInputTextFlags_EnterReturnsTrue);

        if (ImGui::Button("Create") || enter) {
            if (_new_item_buffer[0] != '\0' && _file_ops && _context_node) {
                char path[FileNode::MAX_PATH_LENGTH];
                snprintf(path, sizeof(path), "%s/%s", _context_node->path, _new_item_buffer);
                _file_ops->create_folder(path);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (_show_rename_popup) {
        ImGui::OpenPopup("Rename##popup");
        _show_rename_popup = false;
    }

    if (ImGui::BeginPopupModal("Rename##popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter new name:");
        ImGui::SetKeyboardFocusHere();
        bool enter = ImGui::InputText("##rename", _rename_buffer, sizeof(_rename_buffer), ImGuiInputTextFlags_EnterReturnsTrue);

        if (ImGui::Button("Rename") || enter) {
            if (_rename_buffer[0] != '\0' && _file_ops && _context_node && _context_node->parent) {
                char new_path[FileNode::MAX_PATH_LENGTH];
                snprintf(new_path, sizeof(new_path), "%s/%s", _context_node->parent->path, _rename_buffer);
                if (_context_node->is_directory) {
                    _file_ops->rename_folder(_context_node->path, new_path);
                } else {
                    _file_ops->rename_file(_context_node->path, new_path);
                }
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

}
