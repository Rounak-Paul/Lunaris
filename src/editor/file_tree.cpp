#include "lunaris/editor/file_tree.h"
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
    , _root(nullptr)
    , _on_file_selected(nullptr)
    , _callback_user_data(nullptr) {
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

void FileTree::on_ui() {
    if (!_root) {
        return;
    }

    draw_node(_root, 0, 0);
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

    ImGui::TextUnformatted(node->name);

    ImVec2 row_end = ImGui::GetCursorScreenPos();
    row_end.x = row_start.x + ImGui::GetContentRegionAvail().x + 50.0f;
    row_end.y = row_start.y + line_height;

    bool row_hovered = ImGui::IsMouseHoveringRect(row_start, row_end);
    bool row_clicked = row_hovered && ImGui::IsMouseClicked(0);

    if (row_clicked) {
        if (node->is_directory) {
            is_open = !is_open;
            node->is_expanded = is_open;
        } else if (_on_file_selected) {
            _on_file_selected(node->path, _callback_user_data);
        }
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

}
