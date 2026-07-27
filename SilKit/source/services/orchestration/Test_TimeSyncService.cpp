// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <chrono>
#include <functional>
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

        // this CTor calls CreateTimeSyncService implicitly
        lifecycleService = std::make_unique<LifecycleService>(&participant);
        lifecycleService->SetLifecycleConfiguration(LifecycleConfiguration{OperationMode::Coordinated});
        timeSyncService =
            std::make_unique<TimeSyncService>(&participant, &timeProvider, healthCheckConfig, lifecycleService.get());
        lifecycleService->SetTimeSyncService(timeSyncService.get());
    }

protected: // Methods
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

} // namespace
