// Copyright (c) Vector Informatik GmbH. All rights reserved.
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
