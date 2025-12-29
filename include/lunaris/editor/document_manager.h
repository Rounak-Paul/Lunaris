#pragma once

#include "lunaris/editor/document.h"
#include <cstdint>

namespace lunaris {

class TabBar;
class Theme;

class DocumentManager {
public:
    static constexpr uint32_t MAX_DOCUMENTS = 64;

    DocumentManager();
    ~DocumentManager();

    void set_tab_bar(TabBar* tab_bar) { _tab_bar = tab_bar; }
    void set_theme(Theme* theme) { _theme = theme; }

    DocumentID new_document();
    DocumentID open_document(const char* filepath);
    bool save_document(DocumentID id);
    bool save_document_as(DocumentID id, const char* filepath);
    bool save_active_document();
    bool save_active_document_as(const char* filepath);
    void close_document(DocumentID id);
    void close_active_document();

    void set_active_document(DocumentID id);
    DocumentID get_active_document_id() const { return _active_id; }
    Document* get_active_document();
    const Document* get_active_document() const;

    Document* get_document(DocumentID id);
    const Document* get_document(DocumentID id) const;
    Document* find_by_path(const char* filepath);
    Document* find_by_tab(TabID tab_id);

    uint32_t get_document_count() const { return _document_count; }
    bool has_unsaved_changes() const;

    void on_tab_selected(TabID tab_id);
    void on_tab_closed(TabID tab_id);

    void open_file_dialog();
    void save_file_dialog();

private:
    DocumentID generate_id();
    void sync_tab_modified(DocumentID id);

    Document* _documents[MAX_DOCUMENTS];
    uint32_t _document_count;
    DocumentID _active_id;
    DocumentID _next_id;
    TabBar* _tab_bar;
    Theme* _theme;
};

}
