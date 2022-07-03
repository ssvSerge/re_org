#pragma once
#include <unordered_set>
#include <type_traits>
#include "V100_shared_types.h"

enum CmdFlagEnum
{
    UNHANDLED = 0,
    ATOMIC_FLAG = 1,
    MACRO_FLAG = 2,
    TRANSPORT_TO_SE = 4,
    IGNORE_SE_FEEDBACK = 8,
    SHUTDOWN_SE = 16
};

class CommandItem
{
public:
    _V100_COMMAND_SET Command;
    int CmdFlags;

public:
    class CommandItemComparer
    {
    public:
        using is_transparent = std::true_type;
        bool operator()(const CommandItem &_first, const CommandItem &_second) const
        {
            return std::hash<_V100_COMMAND_SET>()(_first.Command) == std::hash<_V100_COMMAND_SET>()(_second.Command);
        }

        bool operator()(const _V100_COMMAND_SET &_first, const CommandItem &_second) const
        {
            return std::hash<_V100_COMMAND_SET>()(_first) == std::hash<_V100_COMMAND_SET>()(_second.Command);
        }
    };
    bool operator==(const CommandItem& _second) const {
        return _second.Command == Command;
    }
    bool operator==(const _V100_COMMAND_SET& _cmd) const {
        return _cmd == Command;
    }
    CommandItem(_V100_COMMAND_SET _cmd, int flags = CmdFlagEnum::UNHANDLED)
    {
        Command = _cmd;
        CmdFlags = flags;
    }
    CommandItem()
    {
        Command = CMD_NONE;
        CmdFlags = CmdFlagEnum::UNHANDLED;
    }
    operator _V100_COMMAND_SET()
    {
        return Command;
    }
};

struct CommandItemHash{
    size_t operator() (const struct CommandItem& _item) const{
        return std::hash<_V100_COMMAND_SET>()(_item.Command);
    }
    size_t operator() (const _V100_COMMAND_SET& _item) const {
        return std::hash<_V100_COMMAND_SET>()(_item);
    }
};

