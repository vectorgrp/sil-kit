// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SerdesMwSync.hpp"

#include <chrono>

#include "gtest/gtest.h"

using namespace std::chrono_literals;

TEST(MwVAsioSerdes, MwSync_QuantumRequest)
{
    using namespace ib::mw::sync;
    ib::mw::MessageBuffer buffer;

    QuantumRequest in{3ns, 6ns};
    QuantumRequest out{0ns, 0ns};

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.now, out.now);
    EXPECT_EQ(in.duration, out.duration);
}


TEST(MwVAsioSerdes, MwSync_QuantumGrant)
{
    using namespace ib::mw::sync;
    ib::mw::MessageBuffer buffer;

    QuantumGrant in;
    QuantumGrant out{};

    in.grantee = ib::mw::EndpointAddress{3,7};
    in.now = 5ns;
    in.duration = 10ns;
    in.status = QuantumRequestStatus::Granted;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.grantee, out.grantee);
    EXPECT_EQ(in.now, out.now);
    EXPECT_EQ(in.duration, out.duration);
    EXPECT_EQ(in.status, out.status);
}

TEST(MwVAsioSerdes, MwSync_Tick)
{
    using namespace ib::mw::sync;
    ib::mw::MessageBuffer buffer;

    Tick in{7ns, 14ns};
    Tick out{0ns, 0ns};

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.now, out.now);
    EXPECT_EQ(in.duration, out.duration);
}

TEST(MwVAsioSerdes, MwSync_TickDone)
{
    using namespace ib::mw::sync;
    ib::mw::MessageBuffer buffer;

    TickDone in{Tick{7ns, 14ns}};
    TickDone out{Tick{0ns, 0ns}};

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.finishedTick.now, out.finishedTick.now);
    EXPECT_EQ(in.finishedTick.duration, out.finishedTick.duration);
}

TEST(MwVAsioSerdes, MwSync_ParticipantCommand)
{
    using namespace ib::mw::sync;
    ib::mw::MessageBuffer buffer;

    ParticipantCommand in{7, ParticipantCommand::Kind::Initialize};
    ParticipantCommand out{0, ParticipantCommand::Kind::Invalid};

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.participant, out.participant);
    EXPECT_EQ(in.kind, out.kind);
}

TEST(MwVAsioSerdes, MwSync_SystemCommand)
{
    using namespace ib::mw::sync;
    ib::mw::MessageBuffer buffer;

    SystemCommand in{SystemCommand::Kind::Run};
    SystemCommand out{SystemCommand::Kind::Invalid};

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.kind, out.kind);
}

TEST(MwVAsioSerdes, MwSync_ParticipantStatus)
{
    using namespace ib::mw::sync;
    ib::mw::MessageBuffer buffer;

    auto now = std::chrono::system_clock::now();
    decltype(now) nowUs = std::chrono::system_clock::time_point{std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch())};
    decltype(now) outNow;

    ParticipantStatus in;
    ParticipantStatus out{};

    in.participantName = "Name";
    in.state = ParticipantState::Initialized;
    in.enterReason = "Finished initialization";
    in.enterTime = nowUs;
    in.refreshTime = nowUs;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.participantName, out.participantName);
    EXPECT_EQ(in.state, out.state);
    EXPECT_EQ(in.enterReason, out.enterReason);
    EXPECT_EQ(in.enterTime, out.enterTime);
    EXPECT_EQ(in.refreshTime, out.refreshTime);
}