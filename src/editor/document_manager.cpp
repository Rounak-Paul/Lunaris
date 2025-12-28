#include "lunaris/editor/document_manager.h"
#include "lunaris/editor/tab_bar.h"
#include <tinyvk/core/file_dialog.h>
#include <cstring>

namespace lunaris {

DocumentManager::DocumentManager()
    : _document_count(0)
    , _active_id(INVALID_DOCUMENT_ID)
    , _next_id(1)
    , _tab_bar(nullptr)
    , _theme(nullptr) {
}

DocumentManager::~DocumentManager() {
}

DocumentID DocumentManager::generate_id() {
    return _next_id++;
}

DocumentID DocumentManager::new_document() {
    if (_document_count >= MAX_DOCUMENTS) {
        return INVALID_DOCUMENT_ID;
    }

    Document* doc = &_documents[_document_count];
    doc->create_new();
    DocumentID id = generate_id();
    doc->set_id(id);

    if (_tab_bar) {
        TabInfo tab_info;
        tab_info.title = doc->get_title();
        tab_info.filepath = doc->get_filepath();
        tab_info.modified = false;
        tab_info.pinned = false;
        TabID tab_id = _tab_bar->add_tab(tab_info);
        doc->set_tab_id(tab_id);
        _tab_bar->set_active_tab(tab_id);
    }

    ++_document_count;
    _active_id = id;
    return id;
}

DocumentID DocumentManager::open_document(const char* filepath) {
    Document* existing = find_by_path(filepath);
    if (existing) {
        _active_id = existing->get_id();
        if (_tab_bar) {
            _tab_bar->set_active_tab(existing->get_tab_id());
        }
        return existing->get_id();
    }

    if (_document_count >= MAX_DOCUMENTS) {
        return INVALID_DOCUMENT_ID;
    }

    Document* doc = &_documents[_document_count];
    if (!doc->open(filepath)) {
        return INVALID_DOCUMENT_ID;
    }

    DocumentID id = generate_id();
    doc->set_id(id);

    if (_tab_bar) {
        TabInfo tab_info;
        tab_info.title = doc->get_title();
        tab_info.filepath = doc->get_filepath();
        tab_info.modified = false;
        tab_info.pinned = false;
        TabID tab_id = _tab_bar->add_tab(tab_info);
        doc->set_tab_id(tab_id);
        _tab_bar->set_active_tab(tab_id);
    }

    ++_document_count;
    _active_id = id;
    return id;
}

bool DocumentManager::save_document(DocumentID id) {
    Document* doc = get_document(id);
    if (!doc) {
        return false;
    }

    if (!doc->has_file()) {
        return false;
    }

    bool result = doc->save();
    if (result) {
        sync_tab_modified(id);
    }
    return result;
}

bool DocumentManager::save_document_as(DocumentID id, const char* filepath) {
    Document* doc = get_document(id);
    if (!doc) {
        return false;
    }

    bool result = doc->save_as(filepath);
    if (result && _tab_bar) {
        _tab_bar->set_tab_title(doc->get_tab_id(), doc->get_title());
        sync_tab_modified(id);
    }
    return result;
}

bool DocumentManager::save_active_document() {
    return save_document(_active_id);
}

bool DocumentManager::save_active_document_as(const char* filepath) {
    return save_document_as(_active_id, filepath);
}

void DocumentManager::close_document(DocumentID id) {
    int32_t index = -1;
    for (uint32_t i = 0; i < _document_count; ++i) {
        if (_documents[i].get_id() == id) {
            index = static_cast<int32_t>(i);
            break;
        }
    }

    if (index < 0) {
        return;
    }

    Document* doc = &_documents[index];
    TabID closing_tab = doc->get_tab_id();

    if (_tab_bar) {
        _tab_bar->remove_tab(closing_tab);
    }

    doc->close();

    if (static_cast<uint32_t>(index) < _document_count - 1) {
        memmove(&_documents[index], &_documents[index + 1],
                (_document_count - index - 1) * sizeof(Document));
    }
    --_document_count;

    if (_active_id == id) {
        if (_document_count > 0) {
            uint32_t new_idx = (index > 0) ? index - 1 : 0;
            _active_id = _documents[new_idx].get_id();
            if (_tab_bar) {
                _tab_bar->set_active_tab(_documents[new_idx].get_tab_id());
            }
        } else {
            _active_id = INVALID_DOCUMENT_ID;
        }
    }
}

void DocumentManager::close_active_document() {
    close_document(_active_id);
}

void DocumentManager::set_active_document(DocumentID id) {
    Document* doc = get_document(id);
    if (doc) {
        _active_id = id;
        if (_tab_bar) {
            _tab_bar->set_active_tab(doc->get_tab_id());
        }
    }
}

Document* DocumentManager::get_active_document() {
    return get_document(_active_id);
}

const Document* DocumentManager::get_active_document() const {
    return get_document(_active_id);
}

Document* DocumentManager::get_document(DocumentID id) {
    for (uint32_t i = 0; i < _document_count; ++i) {
        if (_documents[i].get_id() == id) {
            return &_documents[i];
        }
    }
    return nullptr;
}

const Document* DocumentManager::get_document(DocumentID id) const {
    for (uint32_t i = 0; i < _document_count; ++i) {
        if (_documents[i].get_id() == id) {
            return &_documents[i];
        }
    }
    return nullptr;
}

Document* DocumentManager::find_by_path(const char* filepath) {
    for (uint32_t i = 0; i < _document_count; ++i) {
        if (strcmp(_documents[i].get_filepath(), filepath) == 0) {
            return &_documents[i];
        }
    }
    return nullptr;
}

Document* DocumentManager::find_by_tab(TabID tab_id) {
    for (uint32_t i = 0; i < _document_count; ++i) {
        if (_documents[i].get_tab_id() == tab_id) {
            return &_documents[i];
        }
    }
    return nullptr;
}

bool DocumentManager::has_unsaved_changes() const {
    for (uint32_t i = 0; i < _document_count; ++i) {
        if (_documents[i].is_modified()) {
            return true;
        }
    }
    return false;
}

void DocumentManager::on_tab_selected(TabID tab_id) {
    Document* doc = find_by_tab(tab_id);
    if (doc) {
        _active_id = doc->get_id();
    }
}

void DocumentManager::on_tab_closed(TabID tab_id) {
    Document* doc = find_by_tab(tab_id);
    if (doc) {
        close_document(doc->get_id());
    }
}

void DocumentManager::open_file_dialog() {
    std::vector<tvk::FileFilter> filters = {
        {"All Files", "*.*"},
        {"C/C++ Files", "*.c;*.cpp;*.cc;*.cxx;*.h;*.hpp;*.hxx"},
        {"Text Files", "*.txt;*.md"},
        {"JSON Files", "*.json"},
    };

    auto result = tvk::FileDialog::OpenFile(filters);
    if (result.has_value()) {
        open_document(result.value().c_str());
    }
}

void DocumentManager::save_file_dialog() {
    Document* doc = get_active_document();
    if (!doc) {
        return;
    }

    std::vector<tvk::FileFilter> filters = {
        {"All Files", "*.*"},
        {"C/C++ Files", "*.c;*.cpp;*.cc;*.cxx;*.h;*.hpp;*.hxx"},
        {"Text Files", "*.txt;*.md"},
    };

    auto result = tvk::FileDialog::SaveFile(filters, doc->has_file() ? doc->get_filepath() : "");
    if (result.has_value()) {
        save_document_as(doc->get_id(), result.value().c_str());
    }
}

void DocumentManager::sync_tab_modified(DocumentID id) {
    Document* doc = get_document(id);
    if (doc && _tab_bar) {
        _tab_bar->set_tab_modified(doc->get_tab_id(), doc->is_modified());
    }
}

}
