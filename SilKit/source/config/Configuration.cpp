/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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
