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

#include "silkit/services/can/all.hpp"

using namespace std::chrono_literals;

namespace {

using namespace SilKit::Core;

auto MakeParticipant(std::string participantName, std::string registryUri, ProtocolVersion version)
    -> std::shared_ptr<IParticipantInternal>
{
    auto cfg = SilKit::Config::ParticipantConfiguration{};
    cfg.participantName = std::move(participantName);
    cfg.middleware.registryUri = std::move(registryUri);

    return std::make_shared<Participant<VAsioConnection>>(std::move(cfg), version);
}

auto MakeRegistry(ProtocolVersion version) -> std::shared_ptr<VAsioRegistry>
{
    auto configPtr = std::make_shared<SilKit::Config::ParticipantConfiguration>();
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
        _registry->StartListening(registryUri);
    }

    void SetupParticipants(std::initializer_list<VersionedParticipant> participants)
    {
        for (auto&& p : participants)
        {
            auto participant = MakeParticipant(p.name, registryUri, p.version);
            _participants.emplace_back(std::move(participant));
        }
    }
    void JoinSimulation()
    {
        for (auto&& participant : _participants)
        {
            participant->JoinSilKitSimulation();
        }
    }
    void ExchangeData()
    {
        std::promise<void> done;
        auto isDone = done.get_future();

        auto&& sender = _participants.at(0);
        auto&& receiver = _participants.at(1);

        auto* recvCan = receiver->CreateCanController("CAN1", "CAN1");
        recvCan->Start();

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

        auto* sendCan = sender->CreateCanController("CAN1", "CAN1");
        sendCan->Start();
        for (auto i = 1; i <= 10; i++)
        {
            const auto frameDataField = std::array<uint8_t, 7>{1, 2, 3, 4, 5, 6, (uint8_t)i};

            SilKit::Services::Can::CanFrame frame;
            frame.canId = 5;
            frame.dataField = SilKit::Util::MakeSpan(frameDataField);
            frame.dlc = static_cast<uint16_t>(frame.dataField.size());
            sendCan->SendFrame(frame);
        }
        auto waitResult = isDone.wait_for(100ms);
        EXPECT_EQ(waitResult, std::future_status::ready);
    }

    std::unique_ptr<SilKit::Tests::SimSystemController> _systemController;
    using ParticipantListT = std::vector<std::shared_ptr<IParticipantInternal>>;
    ParticipantListT _participants;
    std::shared_ptr<VAsioRegistry> _registry;
    const std::string registryUri = "silkit://localhost:8500";
};

TEST_F(ParticipantVersionTest, unsupported_version_connect_to_current)
{
    SetupRegistry(CurrentProtocolVersion());
    //NB: the following causes internal compiler bug in VS2017:
    //SetupParticipants({{"LegacyParticipant", {1,0}}});
    SetupParticipants({VersionedParticipant{"LegacyParticipant", {1, 0}}});
    // do handshake
    EXPECT_THROW(JoinSimulation(), SilKit::ProtocolError);
}

TEST_F(ParticipantVersionTest, Registry30_Participant31_Participant30)
{
    SetupRegistry({3, 0});
    SetupParticipants(
        {VersionedParticipant{"LegacyParticipant", {3, 0}}, VersionedParticipant{"CurrentParticipant", {3, 1}}});
    JoinSimulation();
    ExchangeData();
}

TEST_F(ParticipantVersionTest, Registry31_Participant31_2xParticipant30)
{
    SetupRegistry({3, 1});
    SetupParticipants({VersionedParticipant{"LegacyParticipant", {3, 0}},
                       VersionedParticipant{"CurrentParticipant", {3, 1}},
                       VersionedParticipant{"AnotherParticipant", {3, 0}}});
    JoinSimulation();
    ExchangeData();
}

} // namespace
