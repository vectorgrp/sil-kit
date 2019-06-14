// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ComAdapter.hpp"
#include "ComAdapter_impl.hpp"
#include "ib/cfg/ConfigBuilder.hpp"
#include "CanController.hpp"
#include "FrControllerProxy.hpp"

#include <chrono>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace ib::mw;
using namespace ib::cfg;

struct MockIbConnection
{
    MockIbConnection(Config /*config*/, std::string /*participantName*/) {};

    void joinDomain(uint32_t /*domainId*/) {};

    template<class IbServiceT>
    inline void RegisterIbService(const std::string& /*topicName*/, EndpointId /*endpointId*/, IbServiceT* /*receiver*/) {};

    template<typename IbMessageT>
    void SendIbMessageImpl(EndpointAddress /*from*/, IbMessageT&& /*msg*/) {};

    void WaitForMessageDelivery() {};
    void FlushSendBuffers() {};

    void Run() {};
    void Stop() {};
};

class ComAdapterTest : public testing::Test
{
protected:
    ComAdapterTest()
        : configBuilder("TestBuilder")
        , simulationSetup{configBuilder.SimulationSetup()}
    {
    }

    static auto getClusterParameters() -> ib::sim::fr::ClusterParameters;
    static auto getNodeParameters() -> ib::sim::fr::NodeParameters;

protected:
    ConfigBuilder configBuilder;
    SimulationSetupBuilder& simulationSetup;
};

TEST_F(ComAdapterTest, make_basic_controller)
{
    auto participantName = "CANcontroller";
    auto controllerName = "CAN1";

    simulationSetup.AddParticipant(participantName)
        .AddCan(controllerName)
        .WithLink("LAN0");

    auto config = configBuilder.Build();

    ComAdapter<MockIbConnection> comAdapter(config, participantName);

    auto* canController = comAdapter.CreateCanController(controllerName);

    auto basicCanController = dynamic_cast<ib::sim::can::CanController*>(canController);

    EXPECT_NE(basicCanController, nullptr);
}

TEST_F(ComAdapterTest, make_network_controller)
{
    auto participantName = "FRcontroller";
    auto controllerName = "FR1";
    auto linkName = "P0";

    simulationSetup.AddParticipant(participantName)
        .AddFlexray(controllerName)
        .WithClusterParameters(getClusterParameters())
        .WithNodeParameters(getNodeParameters())
        .WithLink(linkName);

    std::initializer_list<std::string> links{ linkName, "P1" };

    simulationSetup.AddParticipant("NetworkSimulator")
        .AddNetworkSimulator("BusSim")
        .WithLinks(links);

    auto config = configBuilder.Build();

    ComAdapter<MockIbConnection> comAdapter(config, participantName);

    auto* frController = comAdapter.CreateFlexrayController(controllerName);

    auto networkFrController = dynamic_cast<ib::sim::fr::FrControllerProxy*>(frController);

    EXPECT_NE(networkFrController, nullptr);
}

TEST_F(ComAdapterTest, make_basic_and_network_controller)
{
    auto&& p1 = simulationSetup.AddParticipant("P1");
    p1.AddCan("CAN1")
      .WithLink("BUS1");
    p1.AddFlexray("FR1")
      .WithClusterParameters(getClusterParameters())
      .WithNodeParameters(getNodeParameters())
      .WithLink("CLUSTER1");

    auto&& p2 = simulationSetup.AddParticipant("P2");
    p2.AddCan("CAN2")
        .WithLink("BUS1");
    p2.AddFlexray("FR2")
      .WithClusterParameters(getClusterParameters())
      .WithNodeParameters(getNodeParameters())
      .WithLink("CLUSTER1");

    std::initializer_list<std::string> links{ "CLUSTER1" };

    simulationSetup.AddParticipant("NetworkSimulator")
        .AddNetworkSimulator("BusSim")
        .WithLinks(links);

    auto config = configBuilder.Build();

    ComAdapter<MockIbConnection> comAdapter(config, "P1");

    auto* canController = comAdapter.CreateCanController("CAN1");
    auto* frController = comAdapter.CreateFlexrayController("FR1");

    auto basicCanController = dynamic_cast<ib::sim::can::CanController*>(canController);
    auto networkFrController = dynamic_cast<ib::sim::fr::FrControllerProxy*>(frController);

    EXPECT_NE(basicCanController, nullptr);
    EXPECT_NE(networkFrController, nullptr);
}

auto ComAdapterTest::getClusterParameters() -> ib::sim::fr::ClusterParameters
{
    ib::sim::fr::ClusterParameters clusterParams;
    clusterParams.gColdstartAttempts = 8;
    clusterParams.gCycleCountMax = 63;
    clusterParams.gdActionPointOffset = 2;
    clusterParams.gdDynamicSlotIdlePhase = 1;
    clusterParams.gdMiniSlot = 5;
    clusterParams.gdMiniSlotActionPointOffset = 2;
    clusterParams.gdStaticSlot = 31;
    clusterParams.gdSymbolWindow = 1;
    clusterParams.gdSymbolWindowActionPointOffset = 1;
    clusterParams.gdTSSTransmitter = 9;
    clusterParams.gdWakeupTxActive = 60;
    clusterParams.gdWakeupTxIdle = 180;
    clusterParams.gListenNoise = 2;
    clusterParams.gMacroPerCycle = 3636;
    clusterParams.gMaxWithoutClockCorrectionFatal = 2;
    clusterParams.gMaxWithoutClockCorrectionPassive = 2;
    clusterParams.gNumberOfMiniSlots = 291;
    clusterParams.gNumberOfStaticSlots = 70;
    clusterParams.gPayloadLengthStatic = 16;
    clusterParams.gSyncFrameIDCountMax = 15;
    return clusterParams;
}

auto ComAdapterTest::getNodeParameters() -> ib::sim::fr::NodeParameters
{
    ib::sim::fr::NodeParameters nodeParams;
    nodeParams.pAllowHaltDueToClock = 1;
    nodeParams.pAllowPassiveToActive = 0;
    nodeParams.pChannels = ib::sim::fr::Channel::AB;
    nodeParams.pClusterDriftDamping = 2;
    nodeParams.pdAcceptedStartupRange = 212;
    nodeParams.pdListenTimeout = 400162;
    nodeParams.pKeySlotId = 0;
    nodeParams.pKeySlotOnlyEnabled = 0;
    nodeParams.pKeySlotUsedForStartup = 0;
    nodeParams.pKeySlotUsedForSync = 0;
    nodeParams.pLatestTx = 249;
    nodeParams.pMacroInitialOffsetA = 3;
    nodeParams.pMacroInitialOffsetB = 3;
    nodeParams.pMicroInitialOffsetA = 6;
    nodeParams.pMicroInitialOffsetB = 6;
    nodeParams.pMicroPerCycle = 200000;
    nodeParams.pOffsetCorrectionOut = 127;
    nodeParams.pOffsetCorrectionStart = 3632;
    nodeParams.pRateCorrectionOut = 81;
    nodeParams.pWakeupChannel = ib::sim::fr::Channel::A;
    nodeParams.pWakeupPattern = 33;
    nodeParams.pdMicrotick = ib::sim::fr::ClockPeriod::T25NS;
    nodeParams.pSamplesPerMicrotick = 2;
    return nodeParams;
}

} // anonymous namespace
