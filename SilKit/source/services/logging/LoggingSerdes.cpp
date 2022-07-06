// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LoggingSerdes.hpp"


namespace SilKit {
namespace Services {
namespace Logging {
using SilKit::Core::MessageBuffer;

inline MessageBuffer& operator<<(MessageBuffer& buffer, const SourceLoc& sourceLoc)
{
    buffer << sourceLoc.filename
           << sourceLoc.line
           << sourceLoc.funcname;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, SourceLoc& sourceLoc)
{
    buffer >> sourceLoc.filename
           >> sourceLoc.line
           >> sourceLoc.funcname;
    return buffer;
}


inline MessageBuffer& operator<<(MessageBuffer& buffer, const LogMsg& msg)
{
    buffer << msg.logger_name
           << msg.level
           << msg.time
           << msg.source
           << msg.payload;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, LogMsg& msg)
{
    buffer >> msg.logger_name
           >> msg.level
           >> msg.time
           >> msg.source
           >> msg.payload;
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
