// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioConnection.hpp"
#include "ComAdapter.hpp"
#include "ib/cfg/ConfigBuilder.hpp"
#include <chrono>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace std::chrono_literals;

struct CanControllerCallbacks
{
    MOCK_METHOD2(ReceiveMessage, void(ib::sim::can::ICanController*, ib::sim::can::CanMessage));
    MOCK_METHOD2(ReceiveAck, void(ib::sim::can::ICanController*, ib::sim::can::CanTransmitAcknowledge));
};

namespace ib {
namespace sim {
namespace can {
bool operator==(const CanMessage& lhs, const CanMessage& rhs)
{
    return lhs.canId == rhs.canId
        && lhs.dlc == rhs.dlc
        && lhs.dataField == rhs.dataField;
}
} } }

TEST(MwVAsioConnection, does_this_work_at_all)
{
    using namespace ib::mw;

    ib::cfg::ConfigBuilder builder{"TestConfig"};
    auto&& simulationSetup = builder.SimulationSetup();
    simulationSetup.AddParticipant("Participant1")
        .AddCan("CAN_1").WithLink("link");
    simulationSetup.AddParticipant("Participant2")
        .AddCan("CAN_2").WithLink("link");

    auto cfg = builder.Build();


    ComAdapter<VAsioConnection> comAdapter1{cfg, "Participant1"};
    auto* can1 = comAdapter1.CreateCanController("CAN_1");
    CanControllerCallbacks can1callbacks;
    can1->RegisterReceiveMessageHandler([&can1callbacks](auto* controller, auto& message) { can1callbacks.ReceiveMessage(controller, message); });
    can1->RegisterTransmitStatusHandler([&can1callbacks](auto* controller, auto& message) { can1callbacks.ReceiveAck(controller, message); });

    ComAdapter<VAsioConnection> comAdapter2{cfg, "Participant2"};
    auto* can2 = comAdapter2.CreateCanController("CAN_2");
    CanControllerCallbacks can2callbacks;
    can2->RegisterReceiveMessageHandler([&can2callbacks](auto* controller, auto& message) { can2callbacks.ReceiveMessage(controller, message); });
    can2->RegisterTransmitStatusHandler([&can2callbacks](auto* controller, auto& message) { can2callbacks.ReceiveAck(controller, message); });

    ib::mw::VAsioConnectionPeer peer1(&comAdapter1.GetIbConnection());
    ib::mw::VAsioConnectionPeer peer2(&comAdapter2.GetIbConnection());

    peer1.Connect(&peer2);


    ib::sim::can::CanMessage msg;
    std::string payload("Hello From 1!");
    msg.dataField = std::vector<uint8_t>(payload.begin(), payload.end());

    EXPECT_CALL(can2callbacks, ReceiveMessage(can2, msg)).Times(1);
    EXPECT_CALL(can1callbacks, ReceiveAck(can1, testing::A<ib::sim::can::CanTransmitAcknowledge>())).Times(1);


    can1->SendMessage(msg);
}
