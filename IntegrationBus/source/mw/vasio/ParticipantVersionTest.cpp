// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <memory>
#include <chrono>
#include <future>
#include <map>
#include <initializer_list>

#include "gtest/gtest.h"

#include "ProtocolVersion.hpp"
#include "VAsioConnection.hpp"
#include "VAsioRegistry.hpp"
#include "Participant.hpp"

#include "SimSystemController.hpp"

#include "ib/sim/can/all.hpp"

using namespace std::chrono_literals;

namespace {

using namespace ib::mw;

auto MakeParticipant(std::string participantName, ProtocolVersion version) -> std::shared_ptr<IParticipantInternal>
{
    return std::make_shared<Participant<VAsioConnection>>(ib::cfg::ParticipantConfiguration{},
                                                          std::move(participantName), version);
}

auto MakeRegistry(ProtocolVersion version) -> std::shared_ptr<VAsioRegistry>
{
    auto configPtr = std::make_shared<ib::cfg::ParticipantConfiguration>();
    return std::make_shared<VAsioRegistry>(configPtr, version);
}

struct VersionedParticipant
{
    std::string name;
    ProtocolVersion version;
};

class ParticipantVersionTest : public testing::Test
{
protected:
    ParticipantVersionTest() = default;

    void SetupRegistry(ProtocolVersion registryVersion)
    {
        _registry = MakeRegistry(registryVersion);
        _registry->ProvideDomain(_domainId);
    }

    void SetupParticipants(std::initializer_list<VersionedParticipant> participants)
    {
        for (auto&& p : participants)
        {
            auto participant = MakeParticipant(p.name, p.version);
            _participants.emplace_back(std::move(participant));
        }
    }
    void JoinDomain()
    {
        for (auto&& participant : _participants)
        {
            participant->JoinIbDomain(_domainId);
        }
    }
    void ExchangeData()
    {
        std::promise<void> done;
        auto isDone = done.get_future();

        auto&& sender = _participants.at(0);
        auto&& receiver = _participants.at(1);

        auto* recvCan = receiver->CreateCanController("CAN1");
        uint8_t lastIter = 0;
        recvCan->AddFrameHandler([&lastIter, &done](auto*, const auto& event) {
            EXPECT_EQ(event.frame.dataField.size(), 7);
            auto iter = event.frame.dataField.at(6);
            EXPECT_EQ(lastIter + 1, iter);
            lastIter = iter;
            if (lastIter == 10)
            {
                done.set_value();
            }
        });

        auto* sendCan = sender->CreateCanController("CAN1");
        for (auto i = 1; i <= 10; i++)
        {
            ib::sim::can::CanFrame frame;
            frame.canId = 5;
            frame.dataField = {1, 2, 3, 4, 5, 6, (uint8_t)i};
            frame.dlc = frame.dataField.size();
            sendCan->SendFrame(frame);
        }
        auto waitResult = isDone.wait_for(100ms);
        EXPECT_EQ(waitResult, std::future_status::ready);
    }

    std::unique_ptr<ib::test::SimSystemController> _systemController;
    using ParticipantListT = std::vector<std::shared_ptr<IParticipantInternal>>;
    ParticipantListT _participants;
    std::shared_ptr<VAsioRegistry> _registry;
    const std::string _domainId = "vib://localhost:8500";
};

TEST_F(ParticipantVersionTest, unsupported_version_connect_to_current)
{
    SetupRegistry(CurrentProtocolVersion());
    //NB: the following causes internal compiler bug in VS2017:
    //SetupParticipants({{"LegacyParticipant", {1,0}}});
    SetupParticipants({VersionedParticipant{"LegacyParticipant", {1, 0}}});
    // do handshake
    EXPECT_THROW(JoinDomain(), ib::ProtocolError);
}

TEST_F(ParticipantVersionTest, Registry30_Participant31_Participant30)
{
    SetupRegistry({3, 0});
    SetupParticipants(
        {VersionedParticipant{"LegacyParticipant", {3, 0}}, VersionedParticipant{"CurrentParticipant", {3, 1}}});
    JoinDomain();
    ExchangeData();
}

TEST_F(ParticipantVersionTest, Registry31_Participant31_2xParticipant30)
{
    SetupRegistry({3, 1});
    SetupParticipants({VersionedParticipant{"LegacyParticipant", {3, 0}},
                       VersionedParticipant{"CurrentParticipant", {3, 1}},
                       VersionedParticipant{"AnotherParticipant", {3, 0}}});
    JoinDomain();
    ExchangeData();
}

} // namespace
