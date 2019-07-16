// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "ComAdapter.hpp"
#include "ComAdapter_impl.hpp"
#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;

using testing::_;
using testing::A;
using testing::An;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

using namespace ib;

struct MockIbConnection
{
    MockIbConnection(ib::cfg::Config /*config*/, std::string /*participantName*/, ib::mw::ParticipantId /*participantId*/) {};

    void joinDomain(uint32_t /*domainId*/) {};

    template<class IbServiceT>
    inline void RegisterIbService(const std::string& /*topicName*/, mw::EndpointId /*endpointId*/, IbServiceT* /*receiver*/) {};

    template<typename IbMessageT>
    void SendIbMessageImpl(mw::EndpointAddress /*from*/, IbMessageT&& /*msg*/) {};

    void OnAllMessagesDelivered(std::function<void(void)> /*callback*/) {};
    void FlushSendBuffers() {};
    void RegisterNewPeerCallback(std::function<void()> callback) {}
};

class IbConfigExampleITest : public testing::Test
{
protected:
    IbConfigExampleITest() { }

    void VerifyParticipants(const std::vector<cfg::Participant>& participants)
    {
        EXPECT_NE(participants.size(), 0u);

        for (auto&& participant: participants)
        {
            VerifyParticipant(participant.name);
        }
    }

    void VerifyParticipant(const std::string& participantName)
    {
        std::cout << "Verifying participant " << participantName << '\n';
        auto&& participantCfg = cfg::get_by_name(ibConfig.simulationSetup.participants, participantName);

        mw::ComAdapter<MockIbConnection> comAdapter(ibConfig, participantName);

        CreateCanControllers(comAdapter, participantCfg);
        CreateLinControllers(comAdapter, participantCfg);
        CreateEthernetControllers(comAdapter, participantCfg);
        CreateFlexrayControllers(comAdapter, participantCfg);
        CreateIoPorts(comAdapter, participantCfg);
        CreateGenericPubSub(comAdapter, participantCfg);
        GetSyncMaster(comAdapter, participantCfg);
        GetParticipantController(comAdapter, participantCfg);
        GetSystemMonitor(comAdapter, participantCfg);
        GetSystemController(comAdapter, participantCfg);
    }
    void CreateCanControllers(mw::IComAdapter& comAdapter, const cfg::Participant& participantCfg)
    {
        for (auto&& controller : participantCfg.canControllers)
        {
            EXPECT_NE(comAdapter.CreateCanController(controller.name), nullptr);
        }
    }
    void CreateLinControllers(mw::IComAdapter& comAdapter, const cfg::Participant& participantCfg)
    {
        for (auto&& controller : participantCfg.linControllers)
        {
            EXPECT_NE(comAdapter.CreateLinController(controller.name), nullptr);
        }
    }
    void CreateEthernetControllers(mw::IComAdapter& comAdapter, const cfg::Participant& participantCfg)
    {
        for (auto&& controller : participantCfg.ethernetControllers)
        {
            EXPECT_NE(comAdapter.CreateEthController(controller.name), nullptr);
        }
    }
    void CreateFlexrayControllers(mw::IComAdapter& comAdapter, const cfg::Participant& participantCfg)
    {
        for (auto&& controller : participantCfg.flexrayControllers)
        {
            EXPECT_NE(comAdapter.CreateFlexrayController(controller.name), nullptr);
        }
    }
    void CreateIoPorts(mw::IComAdapter& comAdapter, const cfg::Participant& participantCfg)
    {
        for (auto&& port : participantCfg.analogIoPorts)
        {
            if (port.direction == cfg::PortDirection::In)
                EXPECT_NE(comAdapter.CreateAnalogIn(port.name), nullptr);
            else if (port.direction == cfg::PortDirection::Out)
                EXPECT_NE(comAdapter.CreateAnalogOut(port.name), nullptr);
        }
        for (auto&& port : participantCfg.digitalIoPorts)
        {
            if (port.direction == cfg::PortDirection::In)
                EXPECT_NE(comAdapter.CreateDigitalIn(port.name), nullptr);
            else if (port.direction == cfg::PortDirection::Out)
                EXPECT_NE(comAdapter.CreateDigitalOut(port.name), nullptr);
        }
        for (auto&& port : participantCfg.pwmPorts)
        {
            if (port.direction == cfg::PortDirection::In)
                EXPECT_NE(comAdapter.CreatePwmIn(port.name), nullptr);
            else if (port.direction == cfg::PortDirection::Out)
                EXPECT_NE(comAdapter.CreatePwmOut(port.name), nullptr);
        }
        for (auto&& port : participantCfg.patternPorts)
        {
            if (port.direction == cfg::PortDirection::In)
                EXPECT_NE(comAdapter.CreatePatternIn(port.name), nullptr);
            else if (port.direction == cfg::PortDirection::Out)
                EXPECT_NE(comAdapter.CreatePatternOut(port.name), nullptr);
        }
    }
    void CreateGenericPubSub(mw::IComAdapter& comAdapter, const cfg::Participant& participantCfg)
    {
        for (auto&& pub : participantCfg.genericPublishers)
        {
            EXPECT_NE(comAdapter.CreateGenericPublisher(pub.name), nullptr);
        }
        for (auto&& sub : participantCfg.genericSubscribers)
        {
            EXPECT_NE(comAdapter.CreateGenericSubscriber(sub.name), nullptr);
        }
    }
    void GetSyncMaster(mw::IComAdapter& comAdapter, const cfg::Participant& participantCfg)
    {
        if (participantCfg.isSyncMaster)
        {
            EXPECT_NE(comAdapter.GetSyncMaster(), nullptr);
        }
    }
    void GetParticipantController(mw::IComAdapter& comAdapter, const cfg::Participant& /*participantCfg*/)
    {
        EXPECT_NE(comAdapter.GetParticipantController(), nullptr);
        // must be callable repeatedly.
        EXPECT_NE(comAdapter.GetParticipantController(), nullptr);
    }
    void GetSystemMonitor(mw::IComAdapter& comAdapter, const cfg::Participant& /*participantCfg*/)
    {
        EXPECT_NE(comAdapter.GetSystemMonitor(), nullptr);
        // must be callable repeatedly.
        EXPECT_NE(comAdapter.GetSystemMonitor(), nullptr);
    }
    void GetSystemController(mw::IComAdapter& comAdapter, const cfg::Participant& /*participantCfg*/)
    {
        EXPECT_NE(comAdapter.GetSystemController(), nullptr);
        // must be callable repeatedly.
        EXPECT_NE(comAdapter.GetSystemController(), nullptr);
    }


protected:
    ib::cfg::Config ibConfig;
};

TEST_F(IbConfigExampleITest, build_participants_from_IbConfig_Example)
{
    ibConfig = cfg::Config::FromJsonFile("IbConfig_Example.json");
    VerifyParticipants(ibConfig.simulationSetup.participants);
}

TEST_F(IbConfigExampleITest, build_participants_from_IbConfig_IO_Example)
{
    ibConfig = cfg::Config::FromJsonFile("IbConfig_IO-Example.json");
    VerifyParticipants(ibConfig.simulationSetup.participants);
}

TEST_F(IbConfigExampleITest, build_participants_from_IbConfig_CANoe)
{
    ibConfig = cfg::Config::FromJsonFile("IbConfig_Canoe.json");
    VerifyParticipants(ibConfig.simulationSetup.participants);
}

TEST_F(IbConfigExampleITest, build_participants_from_IbConfig_FastRTPS_local)
{
    ibConfig = cfg::Config::FromJsonFile("IbConfig_FastRTPS-local.json");
    VerifyParticipants(ibConfig.simulationSetup.participants);
}

TEST_F(IbConfigExampleITest, build_participants_from_IbConfig_FastRTPS_multicast)
{
    ibConfig = cfg::Config::FromJsonFile("IbConfig_FastRTPS-multicast.json");
    VerifyParticipants(ibConfig.simulationSetup.participants);
}

TEST_F(IbConfigExampleITest, build_participants_from_IbConfig_FastRTPS_unicast)
{
    ibConfig = cfg::Config::FromJsonFile("IbConfig_FastRTPS-unicast.json");
    VerifyParticipants(ibConfig.simulationSetup.participants);
}

TEST_F(IbConfigExampleITest, build_participants_from_IbConfig_FastRTPS_configfile)
{
    ibConfig = cfg::Config::FromJsonFile("IbConfig_FastRTPS-configfile.json");
    VerifyParticipants(ibConfig.simulationSetup.participants);
}

} // anonymous namespace
