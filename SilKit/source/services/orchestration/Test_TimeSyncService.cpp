// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <chrono>
#include <set>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/services/orchestration/string_utils.hpp"
#include "util/functional.hpp"

#include "core/mock/participant/MockParticipant.hpp"
#include "core/service/MockServiceEndpoint.hpp"
#include "config/ParticipantConfiguration.hpp"
#include "services/orchestration/SyncDatatypeUtils.hpp"
#include "services/orchestration/TimeSyncService.hpp"
#include "services/orchestration/LifecycleService.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace SilKit;
using namespace SilKit::Core;
using namespace SilKit::Core::Tests;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Util;

using ::SilKit::Core::Tests::DummyParticipant;


class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD(void, SendMsg,
                (const SilKit::Core::IServiceEndpoint* from, const SilKit::Services::Orchestration::NextSimTask& msg),
                (override));
    MOCK_METHOD(void, SendMsg,
                (const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                 const SilKit::Services::Orchestration::NextSimTask& msg),
                (override));
    MOCK_METHOD(bool, ParticipantHasCapability, (const std::string& participantName, const std::string& capability),
                (const, override));
};


class Test_TimeSyncService : public testing::Test
{
protected: // CTor
    Test_TimeSyncService()
    {
        ON_CALL(participant.mockServiceDiscovery, RegisterServiceDiscoveryHandler)
            .WillByDefault([this](SilKit::Core::Discovery::ServiceDiscoveryHandler handler) {
            serviceDiscoveryHandlers.emplace_back(std::move(handler));
        });

        ON_CALL(participant,
                SendMsg(An<const SilKit::Core::IServiceEndpoint*>(), An<const Services::Orchestration::NextSimTask&>()))
            .WillByDefault(
                [this](const SilKit::Core::IServiceEndpoint* /* from */,
                       const Services::Orchestration::NextSimTask& msg) { sentNextSimTasks.emplace_back(msg); });

        ON_CALL(participant, SendMsg(An<const SilKit::Core::IServiceEndpoint*>(), An<const std::string&>(),
                                     An<const Services::Orchestration::NextSimTask&>()))
            .WillByDefault([this](const SilKit::Core::IServiceEndpoint* /* from */, const std::string& target,
                                  const Services::Orchestration::NextSimTask& msg) {
            sentTargetedNextSimTasks.emplace_back(target, msg);
        });

        ON_CALL(participant, ParticipantHasCapability(_, _)).WillByDefault(Return(true));

        CreateServices();
    }

protected: // Methods
    //! Recreate the services under test, e.g. after changing healthCheckConfig.
    void CreateServices()
    {
        timeSyncService.reset(); // holds a raw pointer to the lifecycle service
        lifecycleService.reset();

        // this CTor calls CreateTimeSyncService implicitly
        lifecycleService = std::make_unique<LifecycleService>(&participant);
        lifecycleService->SetLifecycleConfiguration(LifecycleConfiguration{OperationMode::Coordinated});
        timeSyncService =
            std::make_unique<TimeSyncService>(&participant, &timeProvider, healthCheckConfig, lifecycleService.get());
        lifecycleService->SetTimeSyncService(timeSyncService.get());
    }

    void PrepareLifecycle()
    {
        lifecycleService->SetTimeSyncActive(true);
        (void)lifecycleService->StartLifecycle();

        // Add other participant to lookup
        timeSyncService->GetTimeConfiguration()->AddSynchronizedParticipant("P1");

        // skip uninteresting states
        lifecycleService->NewSystemState(SystemState::ServicesCreated);
        lifecycleService->NewSystemState(SystemState::CommunicationInitializing);
        lifecycleService->NewSystemState(SystemState::CommunicationInitialized);
        lifecycleService->NewSystemState(SystemState::ReadyToRun);
        lifecycleService->NewSystemState(SystemState::Running);
    }

protected:
    // ----------------------------------------
    // Members
    NiceMock<MockServiceEndpoint> endpoint{"P1", "N1", "C1"};

    NiceMock<MockParticipant> participant;
    Config::HealthCheck healthCheckConfig;

    std::vector<SilKit::Core::Discovery::ServiceDiscoveryHandler> serviceDiscoveryHandlers;
    std::vector<SilKit::Services::Orchestration::NextSimTask> sentNextSimTasks;
    std::vector<std::pair<std::string, SilKit::Services::Orchestration::NextSimTask>> sentTargetedNextSimTasks;

    std::unique_ptr<LifecycleService> lifecycleService;
    TimeProvider timeProvider{};
    std::unique_ptr<TimeSyncService> timeSyncService;
};

TEST_F(Test_TimeSyncService, async_simtask_once_without_complete_call)
{
    auto numAsyncTaskCalled{0};
    timeSyncService->SetSimulationStepHandlerAsync([&](auto, auto) { numAsyncTaskCalled++; }, 1ms);

    PrepareLifecycle();

    timeSyncService->ReceiveMsg(&endpoint, {0ms});
    timeSyncService->ReceiveMsg(&endpoint, {1ms});
    timeSyncService->ReceiveMsg(&endpoint, {2ms});
    ASSERT_EQ(numAsyncTaskCalled, 1) << "SimulationStepHandlerAsync should only be called once"
                                     << " until completed with a call to CompleteSimulationStep().";
}

TEST_F(Test_TimeSyncService, async_simtask_complete_lockstep)
{
    auto numAsyncTaskCalled{0};
    timeSyncService->SetSimulationStepHandlerAsync([&](auto, auto) { numAsyncTaskCalled++; }, 1ms);

    PrepareLifecycle();

    timeSyncService->ReceiveMsg(&endpoint, {0ms});
    timeSyncService->CompleteSimulationStep();

    timeSyncService->ReceiveMsg(&endpoint, {1ms});
    timeSyncService->CompleteSimulationStep();

    timeSyncService->ReceiveMsg(&endpoint, {2ms});
    timeSyncService->CompleteSimulationStep();
    ASSERT_EQ(numAsyncTaskCalled, 3)
        << "SimulationStepHandlerAsync should be called in lockstep with calls to CompleteSimulationStep().";
}

TEST_F(Test_TimeSyncService, async_simtask_mismatching_number_of_complete_calls)
{
    // What happens when the User calls CompleteSimulationStep() multiple times?
    auto numAsyncTaskCalled{0};
    timeSyncService->SetSimulationStepHandlerAsync([&](auto, auto) { numAsyncTaskCalled++; }, 1ms);

    PrepareLifecycle();

    timeSyncService->ReceiveMsg(&endpoint, {0ms});
    timeSyncService->CompleteSimulationStep();
    timeSyncService->CompleteSimulationStep();

    timeSyncService->ReceiveMsg(&endpoint, {1ms});
    timeSyncService->CompleteSimulationStep();
    timeSyncService->CompleteSimulationStep();

    timeSyncService->ReceiveMsg(&endpoint, {2ms});
    timeSyncService->CompleteSimulationStep();
    timeSyncService->CompleteSimulationStep();

    ASSERT_EQ(numAsyncTaskCalled, 3) << "Calling too many CompleteSimulationStep() should not wreak havoc";
}

TEST_F(Test_TimeSyncService, switch_handler_kind_before_running_is_allowed)
{
    auto numSyncTaskCalled{0};
    auto numAsyncTaskCalled{0};

    // The last handler set before the participant is running determines the kind
    timeSyncService->SetSimulationStepHandlerAsync([&](auto, auto) { numAsyncTaskCalled++; }, 1ms);
    ASSERT_NO_THROW(timeSyncService->SetSimulationStepHandler([&](auto, auto) { numSyncTaskCalled++; }, 1ms));

    PrepareLifecycle();

    timeSyncService->ReceiveMsg(&endpoint, {0ms});
    timeSyncService->ReceiveMsg(&endpoint, {1ms});
    ASSERT_EQ(numSyncTaskCalled, 2);
    ASSERT_EQ(numAsyncTaskCalled, 0);
}

TEST_F(Test_TimeSyncService, switch_handler_kind_after_start_lifecycle_is_rejected)
{
    timeSyncService->SetSimulationStepHandler([](auto, auto) {}, 1ms);
    lifecycleService->SetTimeSyncActive(true);
    (void)lifecycleService->StartLifecycle();

    ASSERT_NE(lifecycleService->State(), ParticipantState::Running);
    EXPECT_THROW(timeSyncService->SetSimulationStepHandlerAsync([](auto, auto) {}, 1ms), SilKit::StateError);
    EXPECT_NO_THROW(timeSyncService->SetSimulationStepHandler([](auto, auto) {}, 2ms));
}

TEST_F(Test_TimeSyncService, set_handler_of_same_kind_while_running_is_allowed)
{
    timeSyncService->SetSimulationStepHandler([](auto, auto) {}, 1ms);
    PrepareLifecycle();
    ASSERT_EQ(lifecycleService->State(), ParticipantState::Running);
    EXPECT_NO_THROW(timeSyncService->SetSimulationStepHandler([](auto, auto) {}, 2ms));

    CreateServices();
    timeSyncService->SetSimulationStepHandlerAsync([](auto, auto) {}, 1ms);
    PrepareLifecycle();
    ASSERT_EQ(lifecycleService->State(), ParticipantState::Running);
    EXPECT_NO_THROW(timeSyncService->SetSimulationStepHandlerAsync([](auto, auto) {}, 2ms));
}

TEST_F(Test_TimeSyncService, switch_from_sync_to_async_handler_at_runtime_is_rejected)
{
    constexpr auto numSteps = 3;

    auto numSyncTaskCalled{0};
    auto numAsyncTaskCalled{0};
    auto numRejected{0};
    timeSyncService->SetSimulationStepHandler([&](auto, auto) {
        ++numSyncTaskCalled;
        try
        {
            timeSyncService->SetSimulationStepHandlerAsync([&](auto, auto) { numAsyncTaskCalled++; }, 1ms);
        }
        catch (const SilKit::StateError&)
        {
            ++numRejected;
        }
    }, 1ms);

    PrepareLifecycle();

    for (auto i = 0; i < numSteps; ++i)
    {
        timeSyncService->ReceiveMsg(&endpoint, {i * 1ms});
        ASSERT_EQ(numSyncTaskCalled, i + 1);
        ASSERT_EQ(sentNextSimTasks.back().timePoint, (i + 1) * 1ms);
    }
    ASSERT_EQ(numRejected, numSteps);
    ASSERT_EQ(numAsyncTaskCalled, 0);
}

TEST_F(Test_TimeSyncService, switch_from_async_to_sync_handler_at_runtime_is_rejected)
{
    auto numAsyncTaskCalled{0};
    auto numSyncTaskCalled{0};

    timeSyncService->SetSimulationStepHandlerAsync([&](auto, auto) { numAsyncTaskCalled++; }, 1ms);

    PrepareLifecycle();

    timeSyncService->ReceiveMsg(&endpoint, {0ms});
    ASSERT_EQ(numAsyncTaskCalled, 1);

    EXPECT_THROW(timeSyncService->SetSimulationStepHandler([&](auto, auto) { numSyncTaskCalled++; }, 1ms),
                 SilKit::StateError);

    // The async step is still open, so the next step must not start
    timeSyncService->ReceiveMsg(&endpoint, {1ms});
    EXPECT_EQ(numSyncTaskCalled, 0);
    EXPECT_EQ(numAsyncTaskCalled, 1);

    // The rejected switch left the async handler in place
    timeSyncService->CompleteSimulationStep();
    EXPECT_EQ(numSyncTaskCalled, 0);
    EXPECT_EQ(numAsyncTaskCalled, 2);
}

auto MakeTimeSyncServiceDescriptor() -> SilKit::Core::ServiceDescriptor
{
    SilKit::Core::ServiceDescriptor sd;
    sd.SetServiceType(Core::ServiceType::InternalController);
    sd.SetParticipantNameAndComputeId("Hopper");
    sd.SetNetworkName("default");
    sd.SetNetworkType(Config::NetworkType::Undefined);
    sd.SetServiceName("XXX");
    sd.SetServiceId(1234);
    sd.SetSupplementalDataItem(SilKit::Core::Discovery::controllerType, "TimeSyncService");
    sd.SetSupplementalDataItem(SilKit::Core::Discovery::timeSyncActive, "1");

    return sd;
}

TEST_F(Test_TimeSyncService, hop_on_during_simulation_step)
{
    using SilKit::Core::Discovery::ServiceDiscoveryEvent;

    ASSERT_EQ(serviceDiscoveryHandlers.size(), 1u);

    std::vector<std::chrono::nanoseconds> simulationStepNows;
    timeSyncService->SetSimulationStepHandlerAsync(
        [&simulationStepNows](auto now, auto) { simulationStepNows.emplace_back(now); }, 1ms);

    PrepareLifecycle();

    // After 'starting' the first (now == 0ns) NextSimTask message has been sent (which indicates that we're ready to
    // execute the first step)
    ASSERT_EQ(sentNextSimTasks.size(), 1u);
    ASSERT_EQ(sentNextSimTasks[0].timePoint, 0ms);
    // The first simulation step is triggered by received NextSimTask message
    ASSERT_EQ(simulationStepNows.size(), 0u);

    // Trigger the first simulation step
    timeSyncService->ReceiveMsg(&endpoint, {0ms});
    ASSERT_EQ(simulationStepNows.size(), 1u);
    ASSERT_EQ(simulationStepNows[0], 0ms);

    // Complete the first simulation step
    timeSyncService->CompleteSimulationStep();
    // Completion triggers sending the next simulation step message
    ASSERT_EQ(sentNextSimTasks.size(), 2u);
    ASSERT_EQ(sentNextSimTasks[1].timePoint, 1ms);

    // Reception of the next simulation step message (from a remote peer) triggers the second simulation step
    timeSyncService->ReceiveMsg(&endpoint, {1ms});
    ASSERT_EQ(simulationStepNows.size(), 2u);
    ASSERT_EQ(simulationStepNows[1], 1ms);

    // Trigger 'hop-on' which will cause an additional NextSimTask to be sent (with the 'current' timestamp)
    serviceDiscoveryHandlers[0](ServiceDiscoveryEvent::Type::ServiceCreated, MakeTimeSyncServiceDescriptor());
    ASSERT_EQ(sentNextSimTasks.size(), 3u);
    ASSERT_EQ(sentNextSimTasks[2].timePoint, 1ms);

    // Complete the second simulation step
    timeSyncService->CompleteSimulationStep();
    // Completion triggers sending the next simulation step message
    ASSERT_EQ(sentNextSimTasks.size(), 4u);
    ASSERT_EQ(sentNextSimTasks[3].timePoint, 2ms);
}

TEST_F(Test_TimeSyncService, hop_on_outside_simulation_step)
{
    using SilKit::Core::Discovery::ServiceDiscoveryEvent;

    ASSERT_EQ(serviceDiscoveryHandlers.size(), 1u);

    std::vector<std::chrono::nanoseconds> simulationStepNows;
    timeSyncService->SetSimulationStepHandlerAsync(
        [&simulationStepNows](auto now, auto) { simulationStepNows.emplace_back(now); }, 1ms);

    PrepareLifecycle();

    // After 'starting' the first (now == 0ns) NextSimTask message has been sent (which indicates that we're ready to
    // execute the first step)
    ASSERT_EQ(sentNextSimTasks.size(), 1u);
    ASSERT_EQ(sentNextSimTasks[0].timePoint, 0ms);
    // The first simulation step is triggered by received NextSimTask message
    ASSERT_EQ(simulationStepNows.size(), 0u);

    // Trigger the first simulation step
    timeSyncService->ReceiveMsg(&endpoint, {0ms});
    ASSERT_EQ(simulationStepNows.size(), 1u);
    ASSERT_EQ(simulationStepNows[0], 0ms);

    // Complete the first simulation step
    timeSyncService->CompleteSimulationStep();
    // Completion triggers sending the next simulation step message
    ASSERT_EQ(sentNextSimTasks.size(), 2u);
    ASSERT_EQ(sentNextSimTasks[1].timePoint, 1ms);

    // Reception of the next simulation step message (from a remote peer) triggers the second simulation step
    timeSyncService->ReceiveMsg(&endpoint, {1ms});
    ASSERT_EQ(simulationStepNows.size(), 2u);
    ASSERT_EQ(simulationStepNows[1], 1ms);

    // Complete the second simulation step
    timeSyncService->CompleteSimulationStep();
    // Completion triggers sending the next simulation step message
    ASSERT_EQ(sentNextSimTasks.size(), 3u);
    ASSERT_EQ(sentNextSimTasks[2].timePoint, 2ms);

    // Trigger 'hop-on' which will cause an additional NextSimTask to be sent (with the 'next' timestamp)
    serviceDiscoveryHandlers[0](ServiceDiscoveryEvent::Type::ServiceCreated, MakeTimeSyncServiceDescriptor());
    ASSERT_EQ(sentNextSimTasks.size(), 4u);
    ASSERT_EQ(sentNextSimTasks[3].timePoint, 2ms);
}

// A remote TimeSyncService descriptor advertising (or not) that it requests dynamic simulation step
// sizes. timeSyncActive is intentionally left unset because the dynamic-step reaction is independent
// of the time-sync membership handling.
auto MakeDynamicStepDescriptor(const std::string& participantName, bool requestsDynamicStep)
    -> SilKit::Core::ServiceDescriptor
{
    SilKit::Core::ServiceDescriptor sd;
    sd.SetServiceType(Core::ServiceType::InternalController);
    sd.SetParticipantNameAndComputeId(participantName);
    sd.SetNetworkName("default");
    sd.SetNetworkType(Config::NetworkType::Undefined);
    sd.SetServiceName("TimeSyncService");
    sd.SetServiceId(1234);
    sd.SetSupplementalDataItem(SilKit::Core::Discovery::controllerType, "TimeSyncService");
    sd.SetSupplementalDataItem(SilKit::Core::Discovery::timeSyncDynamicStepSize, requestsDynamicStep ? "1" : "0");
    return sd;
}

TEST_F(Test_TimeSyncService, dynamic_step_follower_enables_when_peer_requests)
{
    using SilKit::Core::Discovery::ServiceDiscoveryEvent;
    ASSERT_EQ(serviceDiscoveryHandlers.size(), 1u);

    // Absent config == follow the network: off until a peer requests it.
    timeSyncService->ConfigureDynamicStepSize(std::nullopt);
    EXPECT_FALSE(timeSyncService->GetTimeConfiguration()->IsDynamicStepSizeEnabled());

    // A peer (e.g. a network simulator) advertises the request -> we enable dynamic stepping.
    serviceDiscoveryHandlers[0](ServiceDiscoveryEvent::Type::ServiceCreated,
                                MakeDynamicStepDescriptor("Netsim", true));
    EXPECT_TRUE(timeSyncService->GetTimeConfiguration()->IsDynamicStepSizeEnabled());

    // The requesting peer leaves -> revert to disabled.
    serviceDiscoveryHandlers[0](ServiceDiscoveryEvent::Type::ServiceRemoved,
                                MakeDynamicStepDescriptor("Netsim", true));
    EXPECT_FALSE(timeSyncService->GetTimeConfiguration()->IsDynamicStepSizeEnabled());
}

TEST_F(Test_TimeSyncService, dynamic_step_peer_without_request_does_not_enable)
{
    using SilKit::Core::Discovery::ServiceDiscoveryEvent;

    timeSyncService->ConfigureDynamicStepSize(std::nullopt);
    serviceDiscoveryHandlers[0](ServiceDiscoveryEvent::Type::ServiceCreated,
                                MakeDynamicStepDescriptor("Peer", false));
    EXPECT_FALSE(timeSyncService->GetTimeConfiguration()->IsDynamicStepSizeEnabled());
}

TEST_F(Test_TimeSyncService, dynamic_step_hard_opt_out_ignores_peer_request)
{
    using SilKit::Core::Discovery::ServiceDiscoveryEvent;

    // Explicit false is a hard opt-out: a peer's request must not enable it.
    timeSyncService->ConfigureDynamicStepSize(false);
    serviceDiscoveryHandlers[0](ServiceDiscoveryEvent::Type::ServiceCreated,
                                MakeDynamicStepDescriptor("Netsim", true));
    EXPECT_FALSE(timeSyncService->GetTimeConfiguration()->IsDynamicStepSizeEnabled());
}

TEST_F(Test_TimeSyncService, dynamic_step_driver_enables_without_peers)
{
    // Explicit true enables locally regardless of peers (and also advertises to them).
    timeSyncService->ConfigureDynamicStepSize(true);
    EXPECT_TRUE(timeSyncService->GetTimeConfiguration()->IsDynamicStepSizeEnabled());
}

TEST_F(Test_TimeSyncService, soft_time_limit_warning_reports_the_measured_timeout)
{
    healthCheckConfig.softResponseTimeout = 10ms;
    CreateServices();

    std::mutex mutex;
    std::condition_variable warningArrived;
    std::string warning;

    // The watchdog warns from its own thread while the simulation step handler below is still running.
    auto captureWarning = [&](const Services::Logging::LoggerMessage& msg) {
        if (msg.GetLevel() != Services::Logging::Level::Warn)
        {
            return;
        }

        std::unique_lock<std::mutex> lock{mutex};
        warning = msg.GetMsgString();
        warningArrived.notify_all();
    };
    ON_CALL(participant.logger, ProcessLoggerMessage(_)).WillByDefault(captureWarning);

    auto simulationStep = [&](std::chrono::nanoseconds, std::chrono::nanoseconds) {
        std::unique_lock<std::mutex> lock{mutex};
        warningArrived.wait_for(lock, 30s, [&] { return !warning.empty(); });
    };
    timeSyncService->SetSimulationStepHandler(simulationStep, 1ms);

    PrepareLifecycle();
    timeSyncService->ReceiveMsg(&endpoint, {0ms});

    std::unique_lock<std::mutex> lock{mutex};
    ASSERT_FALSE(warning.empty()) << "the watchdog should have warned about the exceeded soft time limit";
    EXPECT_THAT(warning, HasSubstr("soft time limit"));
    EXPECT_THAT(warning, Not(HasSubstr("{}"))) << "the timeout must be formatted into the message";

    const std::string prefix{"after "};
    const auto timeoutPos = warning.find(prefix);
    ASSERT_NE(timeoutPos, std::string::npos) << warning;

    double timeoutMs{-1.0};
    ASSERT_NO_THROW(timeoutMs = std::stod(warning.substr(timeoutPos + prefix.size()))) << warning;
    EXPECT_GT(timeoutMs, 0.0);
}

// ----------------------------------------
// Central time coordination
// ----------------------------------------

auto MakeSynchronizedDescriptor(const std::string& participantName, bool isTimeCoordinator,
                                bool coordinatedLifecycle = true) -> SilKit::Core::ServiceDescriptor
{
    SilKit::Core::ServiceDescriptor sd;
    sd.SetServiceType(Core::ServiceType::InternalController);
    sd.SetParticipantNameAndComputeId(participantName);
    sd.SetNetworkName("default");
    sd.SetNetworkType(Config::NetworkType::Undefined);
    sd.SetServiceName("TimeSyncService");
    sd.SetServiceId(1234);
    sd.SetSupplementalDataItem(SilKit::Core::Discovery::controllerType, "TimeSyncService");
    sd.SetSupplementalDataItem(SilKit::Core::Discovery::timeSyncActive, "1");
    sd.SetSupplementalDataItem(SilKit::Core::Discovery::timeSyncIsTimeCoordinator, isTimeCoordinator ? "1" : "0");
    sd.SetSupplementalDataItem(SilKit::Core::Discovery::timeSyncLifecycleIsCoordinated,
                               coordinatedLifecycle ? "1" : "0");
    return sd;
}

auto Task(std::chrono::nanoseconds timePoint, std::chrono::nanoseconds duration = 0ns) -> NextSimTask
{
    NextSimTask task;
    task.timePoint = timePoint;
    task.duration = duration;
    return task;
}

class Test_TimeSyncServiceTimeCoordinator : public Test_TimeSyncService
{
protected:
    void StartRunning()
    {
        lifecycleService->SetTimeSyncActive(true);
        (void)lifecycleService->StartLifecycle();
        lifecycleService->NewSystemState(SystemState::ServicesCreated);
        lifecycleService->NewSystemState(SystemState::CommunicationInitializing);
        lifecycleService->NewSystemState(SystemState::CommunicationInitialized);
        lifecycleService->NewSystemState(SystemState::ReadyToRun);
        lifecycleService->NewSystemState(SystemState::Running);
    }

    void Discover(const std::string& participantName, bool isTimeCoordinator,
                  SilKit::Core::Discovery::ServiceDiscoveryEvent::Type type =
                      SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated,
                  bool coordinatedLifecycle = true)
    {
        for (auto&& handler : serviceDiscoveryHandlers)
        {
            handler(type, MakeSynchronizedDescriptor(participantName, isTimeCoordinator, coordinatedLifecycle));
        }
    }

    void DiscoverAutonomousCandidate(const std::string& participantName)
    {
        Discover(participantName, true, SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated, false);
    }

    void Remove(const std::string& participantName, bool isTimeCoordinator)
    {
        Discover(participantName, isTimeCoordinator,
                 SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceRemoved);
    }

    // Collects the messages of all error log entries
    void CaptureErrors()
    {
        ON_CALL(participant.logger, ProcessLoggerMessage(_))
            .WillByDefault([this](const Services::Logging::LoggerMessage& msg) {
            if (msg.GetLevel() == Services::Logging::Level::Error)
            {
                errors.push_back(msg.GetMsgString());
            }
        });
    }

    std::vector<std::string> errors;

    NiceMock<MockServiceEndpoint> timeCoordinatorEndpoint{"TimeCoordinator", "N1", "C1"};
    NiceMock<MockServiceEndpoint> otherEndpoint{"Other", "N1", "C1"};
};

TEST_F(Test_TimeSyncServiceTimeCoordinator, participant_reports_to_time_coordinator_only)
{
    std::vector<std::chrono::nanoseconds> nows;
    timeSyncService->SetSimulationStepHandler([&](auto now, auto) { nows.push_back(now); }, 1ms);

    Discover("Other", false);
    Discover("TimeCoordinator", true);
    // Chosen when all participants replied after ServicesCreated, not on discovery
    EXPECT_FALSE(timeSyncService->GetTimeConfiguration()->IsFollowingTimeCoordinator());

    StartRunning();
    ASSERT_TRUE(timeSyncService->GetTimeConfiguration()->IsFollowingTimeCoordinator());

    // The initial NextSimTask goes to the time coordinator, not to everybody
    EXPECT_TRUE(sentNextSimTasks.empty());
    ASSERT_EQ(sentTargetedNextSimTasks.size(), 1u);
    EXPECT_EQ(sentTargetedNextSimTasks[0].first, "TimeCoordinator");
    EXPECT_EQ(sentTargetedNextSimTasks[0].second.timePoint, 0ms);

    // A broadcast of a peer that does not follow the time coordinator yet is not relevant anymore
    timeSyncService->ReceiveMsg(&otherEndpoint, Task(0ms, 1ms));
    EXPECT_TRUE(nows.empty());

    // The time coordinator's grants drive the simulation steps
    timeSyncService->ReceiveMsg(&timeCoordinatorEndpoint, Task(0ms, 1ms));
    ASSERT_EQ(nows.size(), 1u);
    EXPECT_EQ(nows[0], 0ms);
    ASSERT_EQ(sentTargetedNextSimTasks.size(), 2u);
    EXPECT_EQ(sentTargetedNextSimTasks[1].second.timePoint, 1ms);

    timeSyncService->ReceiveMsg(&timeCoordinatorEndpoint, Task(1ms, 1ms));
    ASSERT_EQ(nows.size(), 2u);
    EXPECT_EQ(nows[1], 1ms);
    EXPECT_TRUE(sentNextSimTasks.empty());
}

TEST_F(Test_TimeSyncServiceTimeCoordinator, participant_resends_next_step_when_switching_to_time_coordinator)
{
    timeSyncService->SetSimulationStepHandler([](auto, auto) {}, 1ms);
    Discover("Other", false);

    StartRunning();
    ASSERT_EQ(sentNextSimTasks.size(), 1u);
    EXPECT_TRUE(sentTargetedNextSimTasks.empty());

    // Time has not started yet: switching is possible, our NextSimTask is reported to the time coordinator again
    Discover("TimeCoordinator", true);
    ASSERT_EQ(sentTargetedNextSimTasks.size(), 1u);
    EXPECT_EQ(sentTargetedNextSimTasks[0].first, "TimeCoordinator");
    EXPECT_EQ(sentTargetedNextSimTasks[0].second.timePoint, 0ms);
}

TEST_F(Test_TimeSyncServiceTimeCoordinator, time_coordinator_joining_after_time_started_aborts)
{
    timeSyncService->SetSimulationStepHandler([](auto, auto) {}, 1ms);
    Discover("Other", false);
    StartRunning();
    timeSyncService->ReceiveMsg(&otherEndpoint, Task(0ms, 1ms));

    EXPECT_CALL(participant.mockSystemController, AbortSimulation()).Times(1);
    Discover("TimeCoordinator", true);
    EXPECT_FALSE(timeSyncService->GetTimeConfiguration()->IsFollowingTimeCoordinator());
}

TEST_F(Test_TimeSyncServiceTimeCoordinator, time_coordinator_grants_time_to_participants)
{
    std::vector<std::chrono::nanoseconds> nows;
    timeSyncService->ConfigureTimeCoordinatorCandidate(true);
    timeSyncService->SetSimulationStepHandler([&](auto now, auto) { nows.push_back(now); }, 1ms);
    Discover("Other", false);

    StartRunning();
    // Nothing on the wire: our own next step goes into the local time coordinator, Other has not reported yet
    EXPECT_TRUE(sentNextSimTasks.empty());
    EXPECT_TRUE(sentTargetedNextSimTasks.empty());
    EXPECT_TRUE(nows.empty());

    // Other reports: both are granted 0ms. Other via the network, the time coordinator itself locally.
    timeSyncService->ReceiveMsg(&otherEndpoint, Task(0ms, 1ms));
    ASSERT_EQ(nows.size(), 1u);
    EXPECT_EQ(nows[0], 0ms);
    ASSERT_EQ(sentTargetedNextSimTasks.size(), 1u);
    EXPECT_EQ(sentTargetedNextSimTasks[0].first, "Other");
    EXPECT_EQ(sentTargetedNextSimTasks[0].second.timePoint, 0ms);

    timeSyncService->ReceiveMsg(&otherEndpoint, Task(1ms, 1ms));
    ASSERT_EQ(nows.size(), 2u);
    EXPECT_EQ(nows[1], 1ms);
    ASSERT_EQ(sentTargetedNextSimTasks.size(), 2u);
    EXPECT_EQ(sentTargetedNextSimTasks[1].second.timePoint, 1ms);
    EXPECT_TRUE(sentNextSimTasks.empty());
}

TEST_F(Test_TimeSyncServiceTimeCoordinator, time_coordinator_continues_when_participant_leaves)
{
    std::vector<std::chrono::nanoseconds> nows;
    timeSyncService->ConfigureTimeCoordinatorCandidate(true);
    timeSyncService->SetSimulationStepHandlerAsync([&](auto now, auto) { nows.push_back(now); }, 1ms);
    Discover("Other", false);

    StartRunning();
    EXPECT_TRUE(nows.empty());

    // Other hops off before reporting: the time coordinator is no longer held back
    Discover("Other", false, SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceRemoved);
    ASSERT_EQ(nows.size(), 1u);
    EXPECT_EQ(nows[0], 0ms);
}

TEST_F(Test_TimeSyncServiceTimeCoordinator, autonomous_time_coordinator_does_not_check_its_own_capabilities)
{
    lifecycleService->SetLifecycleConfiguration(LifecycleConfiguration{OperationMode::Autonomous});
    timeSyncService->ConfigureTimeCoordinatorCandidate(true);
    auto numSteps{0};
    timeSyncService->SetSimulationStepHandlerAsync([&](auto, auto) { ++numSteps; }, 1ms);

    // The connection only knows the capabilities of remote participants
    ON_CALL(participant, ParticipantHasCapability(participant.GetParticipantName(), _)).WillByDefault(Return(false));
    EXPECT_CALL(participant.mockSystemController, AbortSimulation()).Times(0);

    lifecycleService->SetTimeSyncActive(true);
    (void)lifecycleService->StartLifecycle();
    ASSERT_EQ(lifecycleService->State(), ParticipantState::Running);
    EXPECT_EQ(numSteps, 1) << "a time coordinator without other participants grants itself";
}

TEST_F(Test_TimeSyncServiceTimeCoordinator, time_coordinator_aborts_on_participant_without_capability)
{
    timeSyncService->ConfigureTimeCoordinatorCandidate(true);
    timeSyncService->SetSimulationStepHandler([](auto, auto) {}, 1ms);
    ON_CALL(participant, ParticipantHasCapability("Old", _)).WillByDefault(Return(false));
    EXPECT_CALL(participant.mockSystemController, AbortSimulation()).Times(AtLeast(1));
    Discover("Old", false);
    StartRunning();
}

TEST_F(Test_TimeSyncServiceTimeCoordinator, choose_time_coordinator)
{
    using Choice = std::optional<std::string>;
    EXPECT_EQ(TimeSyncService::ChooseTimeCoordinator({}), Choice{""});
    EXPECT_EQ(TimeSyncService::ChooseTimeCoordinator({{"B", true}, {"A", true}, {"C", true}}), Choice{"A"});
    // Coordinated candidates are preferred, autonomous ones are not known to everybody at the barrier
    EXPECT_EQ(TimeSyncService::ChooseTimeCoordinator({{"A", false}, {"B", true}}), Choice{"B"});
    EXPECT_EQ(TimeSyncService::ChooseTimeCoordinator({{"A", false}}), Choice{"A"});
    EXPECT_EQ(TimeSyncService::ChooseTimeCoordinator({{"A", false}, {"B", false}}), std::nullopt);
}

TEST_F(Test_TimeSyncServiceTimeCoordinator, candidate_with_smallest_name_is_chosen)
{
    CaptureErrors();
    timeSyncService->ConfigureTimeCoordinatorCandidate(true); // "MockParticipant"
    timeSyncService->SetSimulationStepHandler([](auto, auto) {}, 1ms);
    EXPECT_CALL(participant.mockSystemController, AbortSimulation()).Times(0);
    Discover("Zulu", true);
    Discover("Alpha", true);

    StartRunning();

    EXPECT_FALSE(timeSyncService->IsTimeCoordinator());
    ASSERT_TRUE(timeSyncService->GetTimeConfiguration()->IsFollowingTimeCoordinator());
    EXPECT_TRUE(sentNextSimTasks.empty());
    ASSERT_EQ(sentTargetedNextSimTasks.size(), 1u);
    EXPECT_EQ(sentTargetedNextSimTasks[0].first, "Alpha");
    EXPECT_TRUE(errors.empty()) << errors.front();
}

TEST_F(Test_TimeSyncServiceTimeCoordinator, own_participant_with_smallest_name_becomes_time_coordinator)
{
    std::vector<std::chrono::nanoseconds> nows;
    timeSyncService->ConfigureTimeCoordinatorCandidate(true); // "MockParticipant"
    timeSyncService->SetSimulationStepHandler([&](auto now, auto) { nows.push_back(now); }, 1ms);
    NiceMock<MockServiceEndpoint> zuluEndpoint{"Zulu", "N1", "C1"};
    EXPECT_CALL(participant.mockSystemController, AbortSimulation()).Times(0);
    Discover("Zulu", true);
    Discover("Other", false);

    StartRunning();
    ASSERT_TRUE(timeSyncService->IsTimeCoordinator());
    EXPECT_TRUE(sentNextSimTasks.empty());
    EXPECT_TRUE(sentTargetedNextSimTasks.empty());

    // Both peers found before the choice are participants of our time coordinator
    timeSyncService->ReceiveMsg(&zuluEndpoint, Task(0ms, 1ms));
    EXPECT_TRUE(nows.empty());
    timeSyncService->ReceiveMsg(&otherEndpoint, Task(0ms, 1ms));
    ASSERT_EQ(nows.size(), 1u);
    std::set<std::string> granted;
    for (auto&& sent : sentTargetedNextSimTasks)
    {
        granted.insert(sent.first);
    }
    EXPECT_EQ(granted, (std::set<std::string>{"Other", "Zulu"}));
}

TEST_F(Test_TimeSyncServiceTimeCoordinator, coordinated_candidate_is_preferred_over_autonomous_candidate)
{
    timeSyncService->SetSimulationStepHandler([](auto, auto) {}, 1ms);
    EXPECT_CALL(participant.mockSystemController, AbortSimulation()).Times(0);
    DiscoverAutonomousCandidate("Alpha");
    Discover("Zulu", true);

    StartRunning();
    ASSERT_EQ(sentTargetedNextSimTasks.size(), 1u);
    EXPECT_EQ(sentTargetedNextSimTasks[0].first, "Zulu");
}

TEST_F(Test_TimeSyncServiceTimeCoordinator, several_autonomous_candidates_abort)
{
    timeSyncService->SetSimulationStepHandler([](auto, auto) {}, 1ms);
    EXPECT_CALL(participant.mockSystemController, AbortSimulation()).Times(AtLeast(1));
    DiscoverAutonomousCandidate("Alpha");
    DiscoverAutonomousCandidate("Zulu");
    StartRunning();
}

TEST_F(Test_TimeSyncServiceTimeCoordinator, candidate_that_was_not_chosen_leaves_without_error)
{
    CaptureErrors();
    timeSyncService->SetSimulationStepHandler([](auto, auto) {}, 1ms);
    Discover("Alpha", true);
    Discover("Zulu", true);
    StartRunning();

    Remove("Zulu", true);
    EXPECT_TRUE(errors.empty()) << errors.front();
    EXPECT_TRUE(timeSyncService->GetTimeConfiguration()->IsFollowingTimeCoordinator());

    // The chosen one leaving is reported (as Debug before Running, so no error here either)
    Remove("Alpha", true);
}

TEST_F(Test_TimeSyncServiceTimeCoordinator, late_candidate_does_not_change_the_choice)
{
    timeSyncService->SetSimulationStepHandler([](auto, auto) {}, 1ms);
    Discover("Bravo", true);
    StartRunning();
    ASSERT_EQ(sentTargetedNextSimTasks.size(), 1u);
    EXPECT_EQ(sentTargetedNextSimTasks[0].first, "Bravo");

    // A larger name does not change the choice
    {
        EXPECT_CALL(participant.mockSystemController, AbortSimulation()).Times(0);
        Discover("Zulu", true);
    }
    // A smaller name would have been chosen: the participants disagree now, abort
    EXPECT_CALL(participant.mockSystemController, AbortSimulation()).Times(1);
    Discover("Alpha", true);
}

} // namespace
