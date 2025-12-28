#pragma once

#include <cstdint>

namespace lunaris {

using CommandID = uint32_t;
constexpr CommandID INVALID_COMMAND_ID = 0;

enum class CommandCategory : uint8_t {
    General,
    File,
    Edit,
    View,
    Navigation,
    Search,
    Debug,
    Plugin
};

struct CommandInfo {
    const char* name;
    const char* description;
    const char* shortcut;
    CommandCategory category;
};

using CommandCallback = void(*)(void* user_data);

class Command {
public:
    Command();
    Command(CommandID id, const CommandInfo& info, CommandCallback callback, void* user_data);

    void execute();

    CommandID get_id() const { return _id; }
    const char* get_name() const { return _info.name; }
    const char* get_description() const { return _info.description; }
    const char* get_shortcut() const { return _info.shortcut; }
    CommandCategory get_category() const { return _info.category; }

private:
    CommandID _id;
    CommandInfo _info;
    CommandCallback _callback;
    void* _user_data;
};

}
