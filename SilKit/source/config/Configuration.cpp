// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Configuration.hpp"

namespace SilKit {
namespace Config {

inline namespace v1 {

bool operator==(const Sink& lhs, const Sink& rhs)
{
    return lhs.type == rhs.type
        && lhs.level == rhs.level
        && lhs.logName == rhs.logName;
}

bool operator==(const Logging& lhs, const Logging& rhs)
{
    return lhs.logFromRemotes == rhs.logFromRemotes
        && lhs.flushLevel == rhs.flushLevel
        && lhs.sinks == rhs.sinks;
}

bool operator==(const TraceSink& lhs, const TraceSink& rhs)
{
    return lhs.name == rhs.name
        && lhs.outputPath == rhs.outputPath
        && lhs.type == rhs.type;
}

bool operator==(const TraceSource& lhs, const TraceSource& rhs)
{
    return lhs.inputPath == rhs.inputPath
        && lhs.type == rhs.type
        && lhs.name == rhs.name;
}

bool operator==(const Replay& lhs, const Replay& rhs)
{
    return lhs.useTraceSource == rhs.useTraceSource
        && lhs.filterMessage == rhs.filterMessage
        && lhs.direction == rhs.direction;
}

bool operator==(const MdfChannel& lhs, const MdfChannel& rhs)
{
    return lhs.channelName == rhs.channelName
        && lhs.channelSource == rhs.channelSource
        && lhs.channelPath == rhs.channelPath
        && lhs.groupName == rhs.groupName
        && lhs.groupSource == rhs.groupSource
        && lhs.groupPath == rhs.groupPath;
}

} // inline namespace v1

} // namespace Config
} // namespace SilKit
