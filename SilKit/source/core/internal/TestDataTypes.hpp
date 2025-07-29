// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <string>
#include <ostream>

#include "MessageBuffer.hpp"
#include "InternalSerdes.hpp"


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


//////////////////////////////////////////////////////////////////////
// String Utils
//////////////////////////////////////////////////////////////////////

namespace Version1 {
inline std::ostream& operator<<(std::ostream& out, const TestMessage& msg)
{
    out << "version1:TestMessage{" << msg.integer << ",\"" << msg.str << "\"}";
    return out;
}
} // namespace Version1

inline namespace Version2 {
inline std::ostream& operator<<(std::ostream& out, const TestMessage& msg)
{
    out << "version2:TestMessage{" << msg.integer << ",\"" << msg.str << "\"}";
    return out;
}
} // namespace Version2

inline std::ostream& operator<<(std::ostream& out, const TestFrameEvent& msg)
{
    out << "TestFrameEvent{" << msg.integer << ",\"" << msg.str << "\"}";
    return out;
}

} // namespace Tests
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
