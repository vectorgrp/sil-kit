// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <chrono>
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

auto SentTimePoints(const std::vector<NextSimTask>& sent) -> std::vector<std::chrono::nanoseconds>
{
    std::vector<std::chrono::nanoseconds> timePoints;
    for (const auto& task : sent)
    {
        timePoints.push_back(task.timePoint);
    }
    return timePoints;
}

TEST_F(Test_TimeSyncService, without_run_ahead_steps_in_lockstep)
{
    std::vector<std::chrono::nanoseconds> simulationStepNows;
    timeSyncService->SetSimulationStepHandler([&](auto now, auto) { simulationStepNows.push_back(now); }, 1ms);

    PrepareLifecycle();
    timeSyncService->ReceiveMsg(&endpoint, {0ms});

    EXPECT_THAT(simulationStepNows, ElementsAre(0ms));
    EXPECT_THAT(SentTimePoints(sentNextSimTasks), ElementsAre(0ms, 1ms));
}

TEST_F(Test_TimeSyncService, run_ahead_waits_for_the_first_announcement_of_peers)
{
    std::vector<std::chrono::nanoseconds> simulationStepNows;
    timeSyncService->SetSimulationStepHandler([&](auto now, auto) { simulationStepNows.push_back(now); }, 1ms);
    timeSyncService->ConfigureMaxRunAhead(4ms);

    PrepareLifecycle();

    EXPECT_TRUE(simulationStepNows.empty());
    EXPECT_THAT(SentTimePoints(sentNextSimTasks), ElementsAre(0ms));
}

TEST_F(Test_TimeSyncService, run_ahead_steps_up_to_the_window_and_thins_out_announcements)
{
    std::vector<std::chrono::nanoseconds> simulationStepNows;
    timeSyncService->SetSimulationStepHandler([&](auto now, auto) { simulationStepNows.push_back(now); }, 1ms);
    timeSyncService->ConfigureMaxRunAhead(4ms);

    PrepareLifecycle();
    timeSyncService->ReceiveMsg(&endpoint, {0ms});

    // The peer is at 0ms, so steps up to 0ms + 4ms run without waiting for it
    EXPECT_THAT(simulationStepNows, ElementsAre(0ms, 1ms, 2ms, 3ms, 4ms));
    // Announced every 2ms (half the window), plus 5ms because we are about to block on the peer
    EXPECT_THAT(SentTimePoints(sentNextSimTasks), ElementsAre(0ms, 2ms, 4ms, 5ms));

    timeSyncService->ReceiveMsg(&endpoint, {3ms});
    EXPECT_THAT(simulationStepNows, ElementsAre(0ms, 1ms, 2ms, 3ms, 4ms, 5ms, 6ms, 7ms));
    EXPECT_THAT(SentTimePoints(sentNextSimTasks), ElementsAre(0ms, 2ms, 4ms, 5ms, 7ms, 8ms));
}

TEST_F(Test_TimeSyncService, run_ahead_of_a_peer_at_the_start_is_not_a_hop_on)
{
    std::vector<std::chrono::nanoseconds> simulationStepNows;
    timeSyncService->SetSimulationStepHandler([&](auto now, auto) { simulationStepNows.push_back(now); }, 1ms);
    timeSyncService->ConfigureMaxRunAhead(4ms);

    PrepareLifecycle();
    // The peer ran ahead within the window before we executed our first step
    timeSyncService->ReceiveMsg(&endpoint, {4ms, 1ms});

    ASSERT_FALSE(simulationStepNows.empty());
    EXPECT_EQ(simulationStepNows.front(), 0ms);
    EXPECT_FALSE(timeSyncService->GetTimeConfiguration()->HoppedOn());
}

auto MakeControllerDescriptor(const std::string& participantName, const std::string& network)
    -> SilKit::Core::ServiceDescriptor
{
    SilKit::Core::ServiceDescriptor sd;
    sd.SetServiceType(Core::ServiceType::Controller);
    sd.SetParticipantNameAndComputeId(participantName);
    sd.SetNetworkName(network);
    sd.SetNetworkType(Config::NetworkType::CAN);
    sd.SetServiceName("Controller");
    sd.SetServiceId(42);
    return sd;
}

TEST_F(Test_TimeSyncService, exact_mode_registers_as_delivery_scheduler)
{
    EXPECT_EQ(timeProvider.GetDeliveryScheduler(), nullptr);
    timeSyncService->ConfigureLatencies(4ms, {});
    EXPECT_EQ(timeProvider.GetDeliveryScheduler(), timeSyncService.get());
    EXPECT_EQ(timeSyncService->LatencyOf("any"), 4ms);
}

TEST_F(Test_TimeSyncService, exact_mode_steps_until_the_step_end_reaches_peer_progress_plus_latency)
{
    std::vector<std::chrono::nanoseconds> simulationStepNows;
    timeSyncService->SetSimulationStepHandler([&](auto now, auto) { simulationStepNows.push_back(now); }, 1ms);
    timeSyncService->ConfigureLatencies(4ms, {});

    PrepareLifecycle();
    timeSyncService->ReceiveMsg(&endpoint, {0ms});

    // Everything the peer sends from 0ms on is due at 4ms or later: steps [0,1) ... [3,4) are complete
    EXPECT_THAT(simulationStepNows, ElementsAre(0ms, 1ms, 2ms, 3ms));
    // Announced every 2ms (half the latency); 4ms is also where we block
    EXPECT_THAT(SentTimePoints(sentNextSimTasks), ElementsAre(0ms, 2ms, 4ms));

    timeSyncService->ReceiveMsg(&endpoint, {3ms});
    EXPECT_THAT(simulationStepNows, ElementsAre(0ms, 1ms, 2ms, 3ms, 4ms, 5ms, 6ms));
    EXPECT_THAT(SentTimePoints(sentNextSimTasks), ElementsAre(0ms, 2ms, 4ms, 6ms, 7ms));
}

TEST_F(Test_TimeSyncService, exact_mode_uses_the_smallest_latency_of_the_peers_networks)
{
    using SilKit::Core::Discovery::ServiceDiscoveryEvent;

    std::vector<std::chrono::nanoseconds> simulationStepNows;
    timeSyncService->SetSimulationStepHandler([&](auto now, auto) { simulationStepNows.push_back(now); }, 1ms);
    timeSyncService->ConfigureLatencies(4ms, {{"CAN1", 2ms}});
    EXPECT_EQ(timeSyncService->LatencyOf("CAN1"), 2ms);
    EXPECT_EQ(timeSyncService->LatencyOf("ETH1"), 4ms);

    serviceDiscoveryHandlers[0](ServiceDiscoveryEvent::Type::ServiceCreated, MakeControllerDescriptor("P1", "ETH1"));
    serviceDiscoveryHandlers[0](ServiceDiscoveryEvent::Type::ServiceCreated, MakeControllerDescriptor("P1", "CAN1"));

    PrepareLifecycle();
    timeSyncService->ReceiveMsg(&endpoint, {0ms});

    EXPECT_THAT(simulationStepNows, ElementsAre(0ms, 1ms));
}

TEST_F(Test_TimeSyncService, exact_mode_delivers_in_the_step_that_contains_the_due_time)
{
    std::vector<std::chrono::nanoseconds> simulationStepNows;
    timeSyncService->SetSimulationStepHandler([&](auto now, auto) { simulationStepNows.push_back(now); }, 1ms);
    timeSyncService->ConfigureLatencies(4ms, {});

    PrepareLifecycle();
    timeSyncService->ReceiveMsg(&endpoint, {0ms});
    ASSERT_THAT(simulationStepNows, ElementsAre(0ms, 1ms, 2ms, 3ms));

    // A message the peer sent at 2ms: its progress is at least 2ms, and the message is due at 6ms
    std::vector<std::chrono::nanoseconds> deliveredAt;
    timeSyncService->Schedule(2ms, 6ms, "P1", "Controller", [&] { deliveredAt.push_back(timeSyncService->Now()); });

    EXPECT_THAT(simulationStepNows, ElementsAre(0ms, 1ms, 2ms, 3ms, 4ms, 5ms));
    EXPECT_TRUE(deliveredAt.empty());

    timeSyncService->ReceiveMsg(&endpoint, {4ms});
    EXPECT_THAT(simulationStepNows, ElementsAre(0ms, 1ms, 2ms, 3ms, 4ms, 5ms, 6ms, 7ms));
    EXPECT_THAT(deliveredAt, ElementsAre(6ms));
}

TEST_F(Test_TimeSyncService, exact_mode_run_ahead_of_a_peer_at_the_start_is_not_a_hop_on)
{
    std::vector<std::chrono::nanoseconds> simulationStepNows;
    timeSyncService->SetSimulationStepHandler([&](auto now, auto) { simulationStepNows.push_back(now); }, 1ms);
    timeSyncService->ConfigureLatencies(4ms, {});

    PrepareLifecycle();
    timeSyncService->ReceiveMsg(&endpoint, {4ms, 1ms});

    ASSERT_FALSE(simulationStepNows.empty());
    EXPECT_EQ(simulationStepNows.front(), 0ms);
    EXPECT_FALSE(timeSyncService->GetTimeConfiguration()->HoppedOn());
}

TEST_F(Test_TimeSyncService, exact_mode_is_disabled_while_dynamic_step_sizes_are_active)
{
    timeSyncService->ConfigureLatencies(4ms, {{"CAN1", 2ms}});
    timeSyncService->ConfigureDynamicStepSize(true);
    EXPECT_EQ(timeSyncService->LatencyOf("CAN1"), 0ns);
    EXPECT_FALSE(timeSyncService->GetTimeConfiguration()->IsExactModeEnabled());
}

TEST_F(Test_TimeSyncService, run_ahead_is_disabled_while_dynamic_step_sizes_are_active)
{
    timeSyncService->ConfigureMaxRunAhead(4ms);
    EXPECT_EQ(timeSyncService->GetTimeConfiguration()->EffectiveMaxRunAhead(), 4ms);

    timeSyncService->ConfigureDynamicStepSize(true);
    EXPECT_EQ(timeSyncService->GetTimeConfiguration()->EffectiveMaxRunAhead(), 0ns);
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

} // namespace
