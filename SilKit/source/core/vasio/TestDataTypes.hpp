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
#pragma once
#include <string>

#include "MessageBuffer.hpp"
#include "InternalSerdes.hpp"

#include "traits/SilKitMsgTraits.hpp"

// Datatypes for testing versioning and renaming of SIL Kit messages.
namespace SilKit {
namespace Core {
namespace Tests {

// a test message that was extended to a new version
namespace Version1 {
struct TestMessage
{
    std::string str{"1"};
    int integer{1};
};
} // namespace Version1

// the newer version of TestMessage
inline namespace Version2 {
struct TestMessage
{
    int integer{2};
    std::string str{"2"};
};
} // namespace Version2

// Assume we had to rename the TestMessage to TestFrameEvent
struct TestFrameEvent
{
    int integer{3};
    std::string str{"renamed"};
};

} // namespace Tests

//////////////////////////////////////////////////////////////////////
// Traits for internal testing data types for wire protocol versioning and evolution
//////////////////////////////////////////////////////////////////////
DefineSilKitMsgTrait_TypeName(SilKit::Core::Tests::Version1, TestMessage)
DefineSilKitMsgTrait_SerdesName(SilKit::Core::Tests::Version1::TestMessage, "TESTMESSAGE");
DefineSilKitMsgTrait_Version(SilKit::Core::Tests::Version1::TestMessage, 1);

DefineSilKitMsgTrait_TypeName(SilKit::Core::Tests::Version2, TestMessage)
DefineSilKitMsgTrait_SerdesName(SilKit::Core::Tests::Version2::TestMessage, "TESTMESSAGE");
DefineSilKitMsgTrait_Version(SilKit::Core::Tests::Version2::TestMessage, 2);

DefineSilKitMsgTrait_TypeName(SilKit::Core::Tests, TestFrameEvent)
//The renamed TestMessage must have the same SerdesName as the previous struct
DefineSilKitMsgTrait_SerdesName(SilKit::Core::Tests::TestFrameEvent, "TESTMESSAGE");
DefineSilKitMsgTrait_Version(SilKit::Core::Tests::TestFrameEvent, 3);


//////////////////////////////////////////////////////////////////////
// Serdes operators
//////////////////////////////////////////////////////////////////////

inline MessageBuffer& operator<<(MessageBuffer& buffer, const Tests::Version1::TestMessage& msg)
{
    buffer << msg.str << msg.integer;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, Tests::Version1::TestMessage& msg)
{
    buffer >> msg.str >> msg.integer;
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const Tests::Version2::TestMessage& msg)
{
    buffer << msg.integer << msg.str;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, Tests::Version2::TestMessage& msg)
{
    buffer >> msg.integer >> msg.str;
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const Tests::TestFrameEvent& msg)
{
    buffer << msg.integer << msg.str;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, Tests::TestFrameEvent& msg)
{
    buffer >> msg.integer >> msg.str;
    return buffer;
}

inline void Deserialize(MessageBuffer& buffer, Tests::Version1::TestMessage& out)
{
    buffer >> out;
}
inline void Deserialize(MessageBuffer& buffer, Tests::Version2::TestMessage& out)
{
    buffer >> out;
}
inline void Deserialize(MessageBuffer& buffer, Tests::TestFrameEvent& out)
{
    buffer >> out;
}

inline void Serialize(MessageBuffer& buffer, const Tests::TestFrameEvent& msg)
{
    buffer << msg;
}
inline void Serialize(MessageBuffer& buffer, const Tests::Version2::TestMessage& msg)
{
    buffer << msg;
}
inline void Serialize(MessageBuffer& buffer, const Tests::Version1::TestMessage& msg)
{
    buffer << msg;
}

} // namespace Core
} // namespace SilKit
