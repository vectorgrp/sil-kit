#include "SerdesMwVAsio.hpp"

#include <chrono>

#include "gtest/gtest.h"

using namespace std::chrono_literals;

namespace ib { namespace mw {
bool operator==(const RegistryMsgHeader& lhs, const RegistryMsgHeader& rhs)
{
	return lhs.preambel == rhs.preambel
		&& lhs.versionHigh == rhs.versionHigh
		&& lhs.versionLow == rhs.versionLow
		;
}

bool operator==(const VAsioPeerUri& lhs, const VAsioPeerUri& rhs)
{
	return lhs.participantId == rhs.participantId
		&& lhs.participantName == rhs.participantName
		&& lhs.acceptorUris == rhs.acceptorUris
		;
}

bool operator==(const ParticipantAnnouncement& lhs, const ParticipantAnnouncement& rhs)
{
	return lhs.capabilities == rhs.capabilities
		&& lhs.messageHeader == rhs.messageHeader
		&& lhs.peerUri == rhs.peerUri
		;
}

bool operator==(const ParticipantAnnouncementReply& lhs, const ParticipantAnnouncementReply& rhs)
{
	return lhs.subscribers == rhs.subscribers;
}

bool operator==(const KnownParticipants& lhs, const KnownParticipants& rhs)
{
	return lhs.messageHeader == rhs.messageHeader
		&& lhs.peerUris == rhs.peerUris
		;
}

}//namespace mw
}//namespace ib

namespace {
using namespace ib::mw;
auto MakePeerUri() -> VAsioPeerUri
{
	VAsioPeerUri in{};
	in.participantId = 1234;
	in.participantName = "Test";
	in.acceptorUris.push_back("local:///tmp/participant1");
	in.acceptorUris.push_back("tcp://localhost");
	return in;
}

TEST(MwVAsioSerdes, vasio_RegistryMsgHeader)
{
	MessageBuffer buffer;
	RegistryMsgHeader in{};
	RegistryMsgHeader out{};

	buffer << in;
	buffer >> out;

	EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, vasio_peerUri)
{
	MessageBuffer buffer;
	VAsioPeerUri in = MakePeerUri();
	VAsioPeerUri out{};

	buffer << in;
	buffer >> out;
	
	EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, vasio_participantAnouncement)
{
	MessageBuffer buffer;
	ParticipantAnnouncement in{}, out{};

	in.messageHeader = RegistryMsgHeader{};
	in.peerUri = MakePeerUri();
	in.capabilities["compression"] = "gzip";
	in.capabilities["serdes"] = "protobuf";

	buffer << in;
	buffer >> out;

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

TEST(MwVAsioSerdes, vasio_VasioMsgSubscribers)
{
	MessageBuffer buffer;
	VAsioMsgSubscriber in = MakeSubscriber();
	VAsioMsgSubscriber out{};

	buffer << in;
	buffer >> out;

	EXPECT_EQ(in, out);
}
TEST(MwVAsioSerdes, vasio_participantAnouncementReply)
{
	MessageBuffer buffer;
	ParticipantAnnouncementReply in{}, out{};

	for(auto i = 0; i < 10; i++)
	{
		in.subscribers.push_back(MakeSubscriber());
	}

	buffer << in;
	buffer >> out;

	EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, vasio_knownParticipants)
{
	MessageBuffer buffer;
	KnownParticipants in{};
	KnownParticipants out{};

	in.messageHeader = RegistryMsgHeader{};
	for(auto i = 0; i < 10; i++)
	{
		VAsioPeerUri vpi;
		vpi.participantId = i;
		vpi.participantName  = "VPI" + std::to_string(i);
		vpi.acceptorUris.push_back("local://localhost");
		in.peerUris.emplace_back(std::move(vpi));
	}

	buffer << in;
	buffer >> out;

	EXPECT_EQ(in, out);
}
} //anonymous 
