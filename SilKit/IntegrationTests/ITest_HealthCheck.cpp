// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Automates the QA test cases 2.11 - 2.13 "Health Monitoring": a participant whose simulation step exceeds the
// configured SoftResponseTimeout logs a warning, one that exceeds the HardResponseTimeout goes to the Error state,
// and the disconnect of a participant is observed by the others.

#include <atomic>
#include <chrono>
#include <future>
#include <string>
#include <thread>

#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "ITestLogFiles.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Services::Orchestration;
using SilKit::IntegrationTests::MakeUniqueLogName;
using SilKit::IntegrationTests::ScopedLogFiles;

class ITest_HealthCheck : public testing::Test
{
protected:
    void SetUp() override
    {
        _registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
        _registryUri = _registry->StartListening("silkit://127.0.0.1:0");
    }

    // A single synchronized participant in autonomous mode does not need a system controller.
    // The first simulation step blocks for stepDuration, all following steps return immediately.
    struct SlowParticipant
    {
        SlowParticipant(const std::string& config, const std::string& registryUri, std::chrono::milliseconds stepDuration)
        {
            participant = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(config),
                                                    "SlowParticipant", registryUri);
            lifecycleService = participant->CreateLifecycleService({OperationMode::Autonomous});
            auto* timeSyncService = lifecycleService->CreateTimeSyncService();

            timeSyncService->SetSimulationStepHandler(
                [this, stepDuration](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
                if (!firstStepDone)
                {
                    std::this_thread::sleep_for(stepDuration);
                    firstStepDone = true;
                }
                if (now >= 10ms && !stopRequested.exchange(true))
                {
                    stepsAfterSlowStepPromise.set_value();
                }
            }, 1ms);
        }

        std::unique_ptr<SilKit::IParticipant> participant;
        ILifecycleService* lifecycleService{nullptr};
        bool firstStepDone{false};
        std::atomic<bool> stopRequested{false};
        std::promise<void> stepsAfterSlowStepPromise;
    };

    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> _registry;
    std::string _registryUri;
};

TEST_F(ITest_HealthCheck, soft_response_timeout_logs_warning_and_simulation_continues)
{
    ScopedLogFiles logFiles{MakeUniqueLogName("ITest_HealthCheck_Soft")};
    const auto config = R"(
HealthCheck:
  SoftResponseTimeout: 50
Logging:
  FlushLevel: Trace
  Sinks:
  - Type: File
    Level: Warn
    LogName: )" + logFiles.Prefix() + "\n";

    {
        SlowParticipant slow{config, _registryUri, 300ms};
        auto finalState = slow.lifecycleService->StartLifecycle();

        auto continued = slow.stepsAfterSlowStepPromise.get_future();
        ASSERT_EQ(continued.wait_for(10s), std::future_status::ready)
            << "The simulation must continue after a soft response timeout";
        EXPECT_EQ(slow.lifecycleService->State(), ParticipantState::Running);

        slow.lifecycleService->Stop("test done");
        ASSERT_EQ(finalState.wait_for(10s), std::future_status::ready);
        EXPECT_EQ(finalState.get(), ParticipantState::Shutdown);
    }

    EXPECT_THAT(logFiles.ReadAll(), testing::HasSubstr("SimStep did not finish within soft time limit"));
}

TEST_F(ITest_HealthCheck, no_warning_if_step_finishes_within_soft_response_timeout)
{
    ScopedLogFiles logFiles{MakeUniqueLogName("ITest_HealthCheck_InTime")};
    const auto config = R"(
HealthCheck:
  SoftResponseTimeout: 2000
  HardResponseTimeout: 4000
Logging:
  FlushLevel: Trace
  Sinks:
  - Type: File
    Level: Warn
    LogName: )" + logFiles.Prefix() + "\n";

    {
        SlowParticipant slow{config, _registryUri, 20ms};
        auto finalState = slow.lifecycleService->StartLifecycle();

        auto continued = slow.stepsAfterSlowStepPromise.get_future();
        ASSERT_EQ(continued.wait_for(10s), std::future_status::ready);

        slow.lifecycleService->Stop("test done");
        ASSERT_EQ(finalState.wait_for(10s), std::future_status::ready);
        EXPECT_EQ(finalState.get(), ParticipantState::Shutdown);
    }

    const auto log = logFiles.ReadAll();
    EXPECT_THAT(log, testing::Not(testing::HasSubstr("soft time limit")));
    EXPECT_THAT(log, testing::Not(testing::HasSubstr("hard time limit")));
}

TEST_F(ITest_HealthCheck, hard_response_timeout_puts_participant_into_error_state)
{
    const auto config = R"(
HealthCheck:
  SoftResponseTimeout: 20
  HardResponseTimeout: 50
)";

    SlowParticipant slow{config, _registryUri, 300ms};

    std::promise<std::string> errorPromise;
    std::atomic<bool> errorSeen{false};
    auto* systemMonitor = slow.participant->CreateSystemMonitor();
    systemMonitor->AddParticipantStatusHandler([&](const ParticipantStatus& status) {
        if (status.participantName == "SlowParticipant" && status.state == ParticipantState::Error
            && !errorSeen.exchange(true))
        {
            errorPromise.set_value(status.enterReason);
        }
    });

    auto finalState = slow.lifecycleService->StartLifecycle();

    auto error = errorPromise.get_future();
    ASSERT_EQ(error.wait_for(10s), std::future_status::ready)
        << "A simulation step exceeding the hard response timeout must put the participant into the Error state";
    EXPECT_THAT(error.get(), testing::HasSubstr("SimStep did not finish within hard time limit"));

    // Stopping from the Error state shuts the participant down.
    slow.lifecycleService->Stop("test done");
    ASSERT_EQ(finalState.wait_for(10s), std::future_status::ready);
    EXPECT_EQ(finalState.get(), ParticipantState::Shutdown);
}

TEST_F(ITest_HealthCheck, disconnect_is_observed_by_other_participants)
{
    auto observer = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""), "Observer",
                                              _registryUri);
    auto* systemMonitor = observer->CreateSystemMonitor();

    std::promise<void> connectedPromise;
    std::promise<void> disconnectedPromise;
    systemMonitor->SetParticipantConnectedHandler([&](const ParticipantConnectionInformation& info) {
        if (info.participantName == "Leaver")
        {
            connectedPromise.set_value();
        }
    });
    systemMonitor->SetParticipantDisconnectedHandler([&](const ParticipantConnectionInformation& info) {
        if (info.participantName == "Leaver")
        {
            disconnectedPromise.set_value();
        }
    });

    auto leaver = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""), "Leaver",
                                            _registryUri);

    ASSERT_EQ(connectedPromise.get_future().wait_for(10s), std::future_status::ready);
    EXPECT_TRUE(systemMonitor->IsParticipantConnected("Leaver"));

    // Destroying the participant closes its connections without any lifecycle transitions.
    leaver.reset();

    ASSERT_EQ(disconnectedPromise.get_future().wait_for(10s), std::future_status::ready)
        << "The observer must be notified when another participant disconnects";
    EXPECT_FALSE(systemMonitor->IsParticipantConnected("Leaver"));
}

} // namespace
