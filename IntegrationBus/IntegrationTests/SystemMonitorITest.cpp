// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <thread>

#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"

#include "SimTestHarness.hpp"
#include "GetTestPid.hpp"
#include "MockParticipantConfiguration.hpp"
#include "VAsioRegistry.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;
using namespace ib::mw;

class SystemMonitorITest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD(void, ParticipantConnectedHandler, (const std::string&), (const));
        MOCK_METHOD(void, ParticipantDisconnectedHandler, (const std::string&), (const));
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
    const std::string firstParticipantName = "First";
    const std::string secondParticipantName = "Second";
    const std::string thirdParticipantName = "Third";

    // Registry
    auto registry = std::make_unique<VAsioRegistry>(ib::cfg::MockParticipantConfiguration());
    registry->ProvideDomain(registryUri);

    // Create the first participant and register the connect and disconnect callbacks
    auto&& firstParticipant =
        ib::CreateParticipant(ib::cfg::MockParticipantConfiguration(), firstParticipantName, registryUri);

    auto* firstSystemMonitor = firstParticipant->GetSystemMonitor();
    firstSystemMonitor->SetParticipantConnectedHandler([this](const std::string& participantName) {
        callbacks.ParticipantConnectedHandler(participantName);
    });
    firstSystemMonitor->SetParticipantDisconnectedHandler([this](const std::string& participantName) {
        callbacks.ParticipantDisconnectedHandler(participantName);
    });

    EXPECT_FALSE(firstSystemMonitor->IsParticipantConnected(secondParticipantName));
    {
        EXPECT_CALL(callbacks, ParticipantConnectedHandler(secondParticipantName)).Times(1);

        // Create the second participant which should trigger the callbacks of the first
        auto&& secondParticipant =
            ib::CreateParticipant(ib::cfg::MockParticipantConfiguration(), secondParticipantName, registryUri);

        auto* secondSystemMonitor = secondParticipant->GetSystemMonitor();
        secondSystemMonitor->SetParticipantConnectedHandler([this](const std::string& participantName) {
            secondCallbacks.ParticipantConnectedHandler(participantName);
        });
        secondSystemMonitor->SetParticipantDisconnectedHandler([this](const std::string& participantName) {
            secondCallbacks.ParticipantDisconnectedHandler(participantName);
        });

        EXPECT_FALSE(firstSystemMonitor->IsParticipantConnected(thirdParticipantName));
        EXPECT_FALSE(secondSystemMonitor->IsParticipantConnected(thirdParticipantName));
        {
            EXPECT_CALL(callbacks, ParticipantConnectedHandler(thirdParticipantName)).Times(1);
            EXPECT_CALL(secondCallbacks, ParticipantConnectedHandler(thirdParticipantName)).Times(1);

            // Create the third participant which should trigger the callbacks of the first and second
            auto&& thirdParticipant =
                ib::CreateParticipant(ib::cfg::MockParticipantConfiguration(), thirdParticipantName, registryUri);

            EXPECT_TRUE(firstSystemMonitor->IsParticipantConnected(thirdParticipantName));
            EXPECT_TRUE(secondSystemMonitor->IsParticipantConnected(thirdParticipantName));

            EXPECT_CALL(callbacks, ParticipantDisconnectedHandler(thirdParticipantName)).Times(1).WillOnce([&] {
                EXPECT_FALSE(firstSystemMonitor->IsParticipantConnected(thirdParticipantName));
            });

            EXPECT_CALL(secondCallbacks, ParticipantDisconnectedHandler(thirdParticipantName)).Times(1).WillOnce([&] {
                EXPECT_FALSE(secondSystemMonitor->IsParticipantConnected(thirdParticipantName));
            });

            // Destroy the third participant
            thirdParticipant.reset();
        }

        EXPECT_CALL(callbacks, ParticipantDisconnectedHandler(secondParticipantName)).Times(1).WillOnce([&] {
            EXPECT_FALSE(firstSystemMonitor->IsParticipantConnected(secondParticipantName));
        });

        // wait for a little while to give the callbacks time to be triggered
        std::this_thread::sleep_for(std::chrono::milliseconds{10});

        // Destroy the second participant
        EXPECT_TRUE(firstSystemMonitor->IsParticipantConnected(secondParticipantName));
        secondParticipant.reset();
    }

    // wait for a little while to give the callbacks time to be triggered
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
}

} // anonymous namespace
