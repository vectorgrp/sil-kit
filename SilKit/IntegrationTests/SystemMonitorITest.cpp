// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <thread>

#include "silkit/services/all.hpp"
#include "silkit/util/functional.hpp"

#include "SimTestHarness.hpp"
#include "GetTestPid.hpp"
#include "ConfigurationTestUtils.hpp"
#include "VAsioRegistry.hpp"
#include "SyncDatatypeUtils.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Core;

class SystemMonitorITest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD(void, ParticipantConnectedHandler,
                    (const SilKit::Services::Orchestration::ParticipantConnectionInformation&),
                    (const));
        MOCK_METHOD(void, ParticipantDisconnectedHandler,
                    (const SilKit::Services::Orchestration::ParticipantConnectionInformation&), (const));
    };

    SystemMonitorITest()
    {
    }

    Callbacks callbacks;
    Callbacks secondCallbacks;
};

// Tests that the service discovery handler fires for created services
// All created should be removed as well if a participant leaves
TEST_F(SystemMonitorITest, discover_services)
{
    auto registryUri = MakeTestRegistryUri();
    const SilKit::Services::Orchestration::ParticipantConnectionInformation& firstParticipantConnection{"First"};
    const SilKit::Services::Orchestration::ParticipantConnectionInformation& secondParticipantConnection{"Second"};
    const SilKit::Services::Orchestration::ParticipantConnectionInformation& thirdParticipantConnection{"Third"};

    // Registry
    auto registry = std::make_unique<VAsioRegistry>(SilKit::Config::MakeEmptyParticipantConfiguration());
    registry->ProvideDomain(registryUri);

    // Create the first participant and register the connect and disconnect callbacks
    auto&& firstParticipant = SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(),
                                                        firstParticipantConnection.participantName, registryUri);

    auto* firstSystemMonitor = firstParticipant->CreateSystemMonitor();
    firstSystemMonitor->SetParticipantConnectedHandler(
        [this](const SilKit::Services::Orchestration::ParticipantConnectionInformation& participantInformation) {
            callbacks.ParticipantConnectedHandler(participantInformation);
    });
    firstSystemMonitor->SetParticipantDisconnectedHandler(
        [this](const SilKit::Services::Orchestration::ParticipantConnectionInformation& participantInformation) {
            callbacks.ParticipantDisconnectedHandler(participantInformation);
    });

    EXPECT_FALSE(firstSystemMonitor->IsParticipantConnected(secondParticipantConnection.participantName));
    {
        EXPECT_CALL(callbacks, ParticipantConnectedHandler(secondParticipantConnection)).Times(1);

        // Create the second participant which should trigger the callbacks of the first
        auto&& secondParticipant = SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(),
                                                             secondParticipantConnection.participantName, registryUri);

        auto* secondSystemMonitor = secondParticipant->CreateSystemMonitor();
        secondSystemMonitor->SetParticipantConnectedHandler(
            [this](const SilKit::Services::Orchestration::ParticipantConnectionInformation&
                       participantConnectionInformation) {
                secondCallbacks.ParticipantConnectedHandler(participantConnectionInformation);
        });
        secondSystemMonitor->SetParticipantDisconnectedHandler(
            [this](const SilKit::Services::Orchestration::ParticipantConnectionInformation&
                       participantConnectionInformation) {
                secondCallbacks.ParticipantDisconnectedHandler(participantConnectionInformation);
        });

        EXPECT_FALSE(firstSystemMonitor->IsParticipantConnected(thirdParticipantConnection.participantName));
        EXPECT_FALSE(secondSystemMonitor->IsParticipantConnected(thirdParticipantConnection.participantName));
        {
            EXPECT_CALL(callbacks, ParticipantConnectedHandler(thirdParticipantConnection)).Times(1);
            EXPECT_CALL(secondCallbacks, ParticipantConnectedHandler(thirdParticipantConnection))
                .Times(1);

            // Create the third participant which should trigger the callbacks of the first and second
            auto&& thirdParticipant =
                SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(),
                                          thirdParticipantConnection.participantName, registryUri);

            EXPECT_TRUE(firstSystemMonitor->IsParticipantConnected(thirdParticipantConnection.participantName));
            EXPECT_TRUE(secondSystemMonitor->IsParticipantConnected(thirdParticipantConnection.participantName));

            EXPECT_CALL(callbacks, ParticipantDisconnectedHandler(thirdParticipantConnection)).Times(1).WillOnce([&] {
                EXPECT_FALSE(firstSystemMonitor->IsParticipantConnected(thirdParticipantConnection.participantName));
            });

            EXPECT_CALL(secondCallbacks, ParticipantDisconnectedHandler(thirdParticipantConnection))
                .Times(1)
                .WillOnce([&] {
                    EXPECT_FALSE(
                        secondSystemMonitor->IsParticipantConnected(thirdParticipantConnection.participantName));
                });

            // Destroy the third participant
            thirdParticipant.reset();
        }

        EXPECT_CALL(callbacks, ParticipantDisconnectedHandler(secondParticipantConnection))
            .Times(1)
            .WillOnce([&] {
            EXPECT_FALSE(firstSystemMonitor->IsParticipantConnected(secondParticipantConnection.participantName));
        });

        // wait for a little while to give the callbacks time to be triggered
        std::this_thread::sleep_for(std::chrono::milliseconds{10});

        // Destroy the second participant
        EXPECT_TRUE(firstSystemMonitor->IsParticipantConnected(secondParticipantConnection.participantName));
        secondParticipant.reset();
    }

    // wait for a little while to give the callbacks time to be triggered
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
}

} // anonymous namespace
