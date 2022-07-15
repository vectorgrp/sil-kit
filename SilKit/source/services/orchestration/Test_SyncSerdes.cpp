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

#include <chrono>

#include "gtest/gtest.h"

#include "SyncSerdes.hpp"

using namespace std::chrono_literals;

namespace {

TEST(MwVAsioSerdes, MwSync_SystemCommand)
{
    using namespace SilKit::Services::Orchestration;
    SilKit::Core::MessageBuffer buffer;

    SystemCommand in{SystemCommand::Kind::AbortSimulation};
    SystemCommand out{SystemCommand::Kind::Invalid};

    Serialize(buffer , in);
    Deserialize(buffer,out);

    EXPECT_EQ(in.kind, out.kind);
}

TEST(MwVAsioSerdes, MwSync_ParticipantStatus)
{
    using namespace SilKit::Services::Orchestration;
    SilKit::Core::MessageBuffer buffer;

    auto now = std::chrono::system_clock::now();
    decltype(now) nowUs = std::chrono::system_clock::time_point{std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch())};

    ParticipantStatus in;
    ParticipantStatus out{};

    in.participantName = "Name";
    in.state = ParticipantState::ReadyToRun;
    in.enterReason = "Finished initialization";
    in.enterTime = nowUs;
    in.refreshTime = nowUs;

    Serialize(buffer , in);
    Deserialize(buffer,out);

    EXPECT_EQ(in.participantName, out.participantName);
    EXPECT_EQ(in.state, out.state);
    EXPECT_EQ(in.enterReason, out.enterReason);
    EXPECT_EQ(in.enterTime, out.enterTime);
    EXPECT_EQ(in.refreshTime, out.refreshTime);
}

} // anonymous namespace

