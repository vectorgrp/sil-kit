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
    buffer << msg.loggerName << msg.level << msg.time << msg.source << msg.payload << msg.keyValues;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, LogMsg& msg)
{
    buffer >> msg.loggerName >> msg.level >> msg.time >> msg.source >> msg.payload;
    if (buffer.RemainingBytesLeft() > 0)
    {
        buffer >> msg.keyValues;
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
