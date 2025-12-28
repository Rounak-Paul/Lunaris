#pragma once

#include "lunaris/core/command.h"

namespace lunaris {

class CommandRegistry {
public:
    static constexpr uint32_t MAX_COMMANDS = 256;

    CommandRegistry();
    ~CommandRegistry();

    CommandID register_command(const CommandInfo& info, CommandCallback callback, void* user_data = nullptr);
    void unregister_command(CommandID id);

    Command* find_command(CommandID id);
    Command* find_command_by_name(const char* name);

    void execute_command(CommandID id);
    void execute_command_by_name(const char* name);

    uint32_t get_command_count() const { return _command_count; }
    Command* get_commands() { return _commands; }

    uint32_t search_commands(const char* query, CommandID* results, uint32_t max_results);

private:
    CommandID generate_id();
    bool matches_query(const Command& cmd, const char* query);

    Command _commands[MAX_COMMANDS];
    uint32_t _command_count;
    CommandID _next_id;
};

}
