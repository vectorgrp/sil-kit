// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>
#include <memory>

#include "NullConnectionParticipant.hpp"
#include "ParticipantConfiguration.hpp"
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

using namespace ib::cfg;

// TODO: Use API getters once they are available
class ParticipantConfigurationExamplesITest : public testing::Test
{
protected:
    ParticipantConfigurationExamplesITest() { }

    void VerifyGetters(std::shared_ptr<IParticipantConfiguration> cfg)
    {
        auto participantConfig = *std::dynamic_pointer_cast<ParticipantConfiguration>(cfg);

        std::cout << "Verifying participant " << participantConfig.participantName << '\n';

        auto participant = ib::mw::CreateNullConnectionParticipantImpl(cfg, participantConfig.participantName, false);

        /*
        GetCanControllers(*participant, participantConfig);
        GetLinControllers(*participant, participantConfig);
        GetEthernetControllers(*participant, participantConfig);
        GetFlexRayControllers(*participant, participantConfig);
        GetDataPublishersSubscribers(*participant, participantConfig);
        GetRpcServersClients(*participant, participantConfig);
        */
    }

    /*
    void GetCanControllers(ib::mw::IParticipant& participant, const ParticipantConfiguration& cfg)
    {
        for (auto&& controller : cfg.canControllers)
        {
            EXPECT_NE(participant.GetCanController(controller.name, "CAN1"), nullptr);
        }
    }
    void GetLinControllers(ib::mw::IParticipant& participant, const ParticipantConfiguration& cfg)
    {
        for (auto&& controller : cfg.linControllers)
        {
            EXPECT_NE(participant.GetLinController(controller.name), nullptr);
        }
    }
    void GetEthernetControllers(ib::mw::IParticipant& participant, const ParticipantConfiguration& cfg)
    {
        for (auto&& controller : cfg.ethernetControllers)
        {
            EXPECT_NE(participant.GetEthController(controller.name), nullptr);
        }
    }
    void GetFlexRayControllers(ib::mw::IParticipant& participant, const ParticipantConfiguration& cfg)
    {
        for (auto&& controller : cfg.flexRayControllers)
        {
            EXPECT_NE(participant.GetFlexrayController(controller.name), nullptr);
        }
    }
    void GetDataPublishersSubscribers(ib::mw::IParticipant& participant, const ParticipantConfiguration& cfg)
    {
        for (auto&& pub : cfg.dataPublishers)
        {
            EXPECT_NE(participant.GetDataPublisher(pub.name), nullptr);
        }
        for (auto&& sub : cfg.dataSubscribers)
        {
            EXPECT_NE(participant.GetDataSubscriber(sub.name), nullptr);
        }
    }
    void GetRpcServersClients(ib::mw::IParticipant& participant, const ParticipantConfiguration& cfg)
    {
        for (auto&& server : cfg.rpcServers)
        {
            EXPECT_NE(participant.GetRpcServer(pub.name), nullptr);
        }
        for (auto&& client : cfg.rpcClients)
        {
            EXPECT_NE(participant.GetRpcClient(sub.name), nullptr);
        }
    }
    */

};

TEST_F(ParticipantConfigurationExamplesITest, DISABLED_throw_if_logging_is_configured_without_filename)
{
    EXPECT_THROW(ib::cfg::ParticipantConfigurationFromFile("ParticipantConfiguration_Logging_Without_File.json"), ib::ConfigurationError);
}

TEST_F(ParticipantConfigurationExamplesITest, DISABLED_minimal_configuration_file)
{
    auto cfg = ib::cfg::ParticipantConfigurationFromFile("ParticipantConfiguration_Minimal.json");
    VerifyGetters(cfg);
}

TEST_F(ParticipantConfigurationExamplesITest, DISABLED_full_configuration_file)
{
    auto cfg = ib::cfg::ParticipantConfigurationFromFile("ParticipantConfiguration_Full.json");
    VerifyGetters(cfg);
}

} // anonymous namespace
