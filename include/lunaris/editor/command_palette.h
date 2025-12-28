#pragma once

#include "lunaris/core/command.h"

namespace lunaris {

class CommandRegistry;
class Theme;

class CommandPalette {
public:
    static constexpr uint32_t MAX_RESULTS = 32;
    static constexpr float WIDTH = 500.0f;
    static constexpr float MAX_HEIGHT = 400.0f;
    static constexpr float ITEM_HEIGHT = 32.0f;

    CommandPalette();
    ~CommandPalette();

    void set_command_registry(CommandRegistry* registry) { _registry = registry; }
    void set_theme(Theme* theme) { _theme = theme; }

    void open();
    void close();
    void toggle();

    bool is_open() const { return _is_open; }

    void on_ui();

private:
    void update_search();
    void draw_input();
    void draw_results();
    void execute_selected();

    CommandRegistry* _registry;
    Theme* _theme;

    bool _is_open;
    bool _focus_input;
    char _search_buffer[256];
    CommandID _results[MAX_RESULTS];
    uint32_t _result_count;
    uint32_t _selected_index;
};

}
