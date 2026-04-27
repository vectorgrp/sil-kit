// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "LoggingSerdes.hpp"


namespace SilKit {
namespace Services {
namespace Logging {
using SilKit::Core::MessageBuffer;

inline MessageBuffer& operator<<(MessageBuffer& buffer, const SourceLoc& sourceLoc)
{
    buffer << sourceLoc.filename << sourceLoc.line << sourceLoc.funcname;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, SourceLoc& sourceLoc)
{
    buffer >> sourceLoc.filename >> sourceLoc.line >> sourceLoc.funcname;
    return buffer;
}


inline MessageBuffer& operator<<(MessageBuffer& buffer, const LogMsg& msg)
{
    buffer << msg.loggerName << msg.level << msg.time << msg.source << msg.payload << msg.keyValues
           << SilKit::Services::Logging::to_string(msg.topic);
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, LogMsg& msg)
{
    buffer >> msg.loggerName >> msg.level >> msg.time >> msg.source >> msg.payload;
    if (buffer.RemainingBytesLeft() > 0)
    {
        buffer >> msg.keyValues;
    }
    if (buffer.RemainingBytesLeft() > 0)
    {
        std::string tempTopic;
        buffer >> tempTopic;
        msg.topic = from_topic_string(tempTopic);
    }
    return buffer;
}


void Serialize(MessageBuffer& buffer, const LogMsg& msg)
{
    buffer << msg;
    return;
}

void Deserialize(MessageBuffer& buffer, LogMsg& out)
{
    buffer >> out;
}
} // namespace Logging
} // namespace Services
} // namespace SilKit
