// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once
#include <string>

#include "MessageBuffer.hpp"
#include "InternalSerdes.hpp"

#include "traits/IbMsgTraits.hpp"

//Datatypes for testing versioning and renaming of IB messages.
namespace ib
{
namespace mw
{
namespace test
{

// a test message that was extended to a new version 
namespace version1
{
struct TestMessage
{
    std::string str{"1"};
    int integer{1};
};
//Printing helper
} // version1

// the newer version of TestMessage
inline namespace version2
{
struct TestMessage
{
    int integer{2};
    std::string str{"2"};
};
} // version2

// Assume we had to rename the TestMessage to TestFrameEvent
struct TestFrameEvent
{
    int integer{3};
    std::string str{"renamed"};
};

} // test

//////////////////////////////////////////////////////////////////////
// Traits for internal testing data types for wire protocol versioning and evolution
//////////////////////////////////////////////////////////////////////
DefineIbMsgTrait_TypeName(ib::mw::test::version1, TestMessage)
DefineIbMsgTrait_SerdesName(ib::mw::test::version1::TestMessage, "TESTMESSAGE");
DefineIbMsgTrait_Version(ib::mw::test::version1::TestMessage, 1);

DefineIbMsgTrait_TypeName(ib::mw::test::version2, TestMessage)
DefineIbMsgTrait_SerdesName(ib::mw::test::version2::TestMessage, "TESTMESSAGE");
DefineIbMsgTrait_Version(ib::mw::test::version2::TestMessage, 2);

DefineIbMsgTrait_TypeName(ib::mw::test, TestFrameEvent)
//The renamed TestMessage must have the same SerdesName as the previous struct
DefineIbMsgTrait_SerdesName(ib::mw::test::TestFrameEvent, "TESTMESSAGE");
DefineIbMsgTrait_Version(ib::mw::test::TestFrameEvent, 3);


//////////////////////////////////////////////////////////////////////
// Serdes operators
//////////////////////////////////////////////////////////////////////

inline MessageBuffer& operator<<(MessageBuffer& buffer, const test::version1::TestMessage& msg)
{
    buffer << msg.str << msg.integer;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, test::version1::TestMessage& msg)
{
    buffer >> msg.str >> msg.integer;
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const test::version2::TestMessage& msg)
{
    buffer << msg.integer << msg.str;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, test::version2::TestMessage& msg)
{
    buffer >> msg.integer >> msg.str;
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const test::TestFrameEvent& msg)
{
    buffer << msg.integer << msg.str;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, test::TestFrameEvent& msg)
{
    buffer >> msg.integer >> msg.str;
    return buffer;
}

inline void Deserialize(MessageBuffer& buffer, test::version1::TestMessage& out)
{
    buffer >> out;
}
inline void Deserialize(MessageBuffer& buffer, test::version2::TestMessage& out)
{
    buffer >> out;
}
inline void Deserialize(MessageBuffer& buffer, test::TestFrameEvent& out)
{
    buffer >> out;
}

inline void Serialize(MessageBuffer& buffer, const test::TestFrameEvent& msg)
{
    buffer << msg;
    return;
}

inline auto Serialize(MessageBuffer& buffer, const test::version2::TestMessage& msg)
{
    buffer << msg;
    return;
}

inline auto Serialize(MessageBuffer& buffer, const test::version1::TestMessage& msg)
{
    buffer << msg;
    return;
}

} // mw
} // ib
