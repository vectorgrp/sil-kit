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
#include "VAsioSerdes.hpp"

#include <chrono>

#include "gtest/gtest.h"

using namespace std::chrono_literals;

namespace SilKit {
namespace Core {

bool operator==(const VAsioPeerInfo& lhs, const VAsioPeerInfo& rhs)
{
    return lhs.participantId == rhs.participantId && lhs.participantName == rhs.participantName
           && lhs.acceptorUris == rhs.acceptorUris && lhs.capabilities == rhs.capabilities;
}

bool operator==(const ParticipantAnnouncement& lhs, const ParticipantAnnouncement& rhs)
{
    return lhs.messageHeader == rhs.messageHeader && lhs.peerInfo == rhs.peerInfo;
}

bool operator==(const ParticipantAnnouncementReply& lhs, const ParticipantAnnouncementReply& rhs)
{
    return lhs.subscribers == rhs.subscribers;
}

bool operator==(const KnownParticipants& lhs, const KnownParticipants& rhs)
{
    return lhs.messageHeader == rhs.messageHeader && lhs.peerInfos == rhs.peerInfos;
}

} // namespace Core
} // namespace SilKit

namespace {

using namespace SilKit::Core;

auto MakePeerInfo() -> VAsioPeerInfo
{
    VAsioPeerInfo in{};
    in.participantId = 1234;
    in.participantName = "Test";
    in.acceptorUris.push_back("local:///tmp/participant1");
    in.acceptorUris.push_back("tcp://localhost");
    in.capabilities = "compression=gzip;future-proof=true";
    return in;
}

TEST(Test_VAsioSerdes, vasio_participantAnouncement)
{
    MessageBuffer buffer;
    ParticipantAnnouncement in{}, out{};

    in.messageHeader = RegistryMsgHeader{};
    in.peerInfo = MakePeerInfo();

    ASSERT_NE(in.messageHeader.versionHigh, 0);
    ASSERT_EQ(in.peerInfo.participantName, "Test");
    Serialize(buffer, in);
    Deserialize(buffer, out);

    EXPECT_EQ(in, out);
}

auto MakeSubscriber() -> VAsioMsgSubscriber
{
    VAsioMsgSubscriber out;
    out.msgTypeName = "TypeName";
    out.networkName = "networkName";
    out.receiverIdx = 1234;
    out.version = 12;
    return out;
}

TEST(Test_VAsioSerdes, vasio_VasioMsgSubscribers)
{
    MessageBuffer buffer;
    VAsioMsgSubscriber in = MakeSubscriber();
    VAsioMsgSubscriber out{};

    Serialize(buffer, in);
    Deserialize(buffer, out);

    EXPECT_EQ(in, out);
}
TEST(Test_VAsioSerdes, vasio_participantAnouncementReply)
{
    MessageBuffer buffer;
    ParticipantAnnouncementReply in{}, out{};

    for (auto i = 0; i < 10; i++)
    {
        in.subscribers.push_back(MakeSubscriber());
    }

    Serialize(buffer, in);
    Deserialize(buffer, out);

    EXPECT_EQ(in, out);
}

TEST(Test_VAsioSerdes, vasio_knownParticipants)
{
    MessageBuffer buffer;
    KnownParticipants in{};
    KnownParticipants out{};

    in.messageHeader = RegistryMsgHeader{};
    for (auto i = 0; i < 10; i++)
    {
        VAsioPeerInfo vpi;
        vpi.participantId = i;
        vpi.participantName = "VPI" + std::to_string(i);
        vpi.acceptorUris.push_back("local://localhost");
        in.peerInfos.emplace_back(std::move(vpi));
    }

    Serialize(buffer, in);
    Deserialize(buffer, out);

    EXPECT_EQ(in, out);
}

} // namespace
