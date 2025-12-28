#include "lunaris/core/command_registry.h"
#include <cstring>
#include <cctype>

namespace lunaris {

CommandRegistry::CommandRegistry()
    : _command_count(0)
    , _next_id(1) {
}

CommandRegistry::~CommandRegistry() {
}

CommandID CommandRegistry::generate_id() {
    return _next_id++;
}

CommandID CommandRegistry::register_command(const CommandInfo& info, CommandCallback callback, void* user_data) {
    if (_command_count >= MAX_COMMANDS) {
        return INVALID_COMMAND_ID;
    }

    CommandID id = generate_id();
    _commands[_command_count] = Command(id, info, callback, user_data);
    _command_count++;
    return id;
}

void CommandRegistry::unregister_command(CommandID id) {
    for (uint32_t i = 0; i < _command_count; ++i) {
        if (_commands[i].get_id() == id) {
            for (uint32_t j = i; j < _command_count - 1; ++j) {
                _commands[j] = _commands[j + 1];
            }
            _command_count--;
            return;
        }
    }
}

Command* CommandRegistry::find_command(CommandID id) {
    for (uint32_t i = 0; i < _command_count; ++i) {
        if (_commands[i].get_id() == id) {
            return &_commands[i];
        }
    }
    return nullptr;
}

Command* CommandRegistry::find_command_by_name(const char* name) {
    for (uint32_t i = 0; i < _command_count; ++i) {
        if (_commands[i].get_name() && strcmp(_commands[i].get_name(), name) == 0) {
            return &_commands[i];
        }
    }
    return nullptr;
}

void CommandRegistry::execute_command(CommandID id) {
    Command* cmd = find_command(id);
    if (cmd) {
        cmd->execute();
    }
}

void CommandRegistry::execute_command_by_name(const char* name) {
    Command* cmd = find_command_by_name(name);
    if (cmd) {
        cmd->execute();
    }
}

bool CommandRegistry::matches_query(const Command& cmd, const char* query) {
    if (!query || query[0] == '\0') {
        return true;
    }

    const char* name = cmd.get_name();
    if (!name) {
        return false;
    }

    size_t query_len = strlen(query);
    size_t name_len = strlen(name);

    size_t q = 0;
    for (size_t n = 0; n < name_len && q < query_len; ++n) {
        if (tolower(static_cast<unsigned char>(name[n])) == 
            tolower(static_cast<unsigned char>(query[q]))) {
            ++q;
        }
    }

    return q == query_len;
}

uint32_t CommandRegistry::search_commands(const char* query, CommandID* results, uint32_t max_results) {
    uint32_t count = 0;

    for (uint32_t i = 0; i < _command_count && count < max_results; ++i) {
        if (matches_query(_commands[i], query)) {
            results[count++] = _commands[i].get_id();
        }
    }

    return count;
}

}
