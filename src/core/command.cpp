#include "lunaris/core/command.h"
#include <cstring>

namespace lunaris {

Command::Command()
    : _id(INVALID_COMMAND_ID)
    , _callback(nullptr)
    , _user_data(nullptr) {
    _info.name = nullptr;
    _info.description = nullptr;
    _info.shortcut = nullptr;
    _info.category = CommandCategory::General;
}

Command::Command(CommandID id, const CommandInfo& info, CommandCallback callback, void* user_data)
    : _id(id)
    , _info(info)
    , _callback(callback)
    , _user_data(user_data) {
}

void Command::execute() {
    if (_callback) {
        _callback(_user_data);
    }
}

}
