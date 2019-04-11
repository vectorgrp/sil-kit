// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/mw/logging/LoggingDatatypes.hpp"

namespace ib {
namespace mw {
namespace logging {

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


} // namespace logging
} // namespace mw
} // namespace ib
