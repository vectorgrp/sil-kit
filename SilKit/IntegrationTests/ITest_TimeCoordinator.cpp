// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "ITestFixture.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"

using namespace std::chrono_literals;

namespace std {
namespace chrono {
inline std::ostream& operator<<(std::ostream& out, const nanoseconds& timestamp)
{
    return out << timestamp.count() << "ns";
}
} // namespace chrono
} // namespace std

namespace {

using namespace SilKit::Tests;
using namespace SilKit::Services::Orchestration;

using Steps = std::vector<std::pair<std::chrono::nanoseconds, std::chrono::nanoseconds>>;

struct ParticipantParams
{
    std::string name;
    std::chrono::nanoseconds stepSize{1ms};
    bool isTimeCoordinator{false};
    bool asyncStepHandler{false};
    std::optional<bool> dynamicSimulationStep{};

    // Result: executed simulation steps (now, duration)
    Steps steps{};
};

auto MakeParticipantConfiguration(const ParticipantParams& params) -> std::string
{
    std::string timeSynchronization;
    if (params.isTimeCoordinator)
    {
        timeSynchronization += R"("TimeCoordinator":true)";
    }
    if (params.dynamicSimulationStep.has_value())
    {
        timeSynchronization += timeSynchronization.empty() ? "" : ",";
        timeSynchronization += R"("DynamicSimulationStep":)";
        timeSynchronization += params.dynamicSimulationStep.value() ? "true" : "false";
    }
    return R"({"Logging":{"Sinks":[{"Type":"File","Level":"Trace","LogName":"TimeCoordinator_)" + params.name
           + R"("}]},"Experimental":{"TimeSynchronization":{)" + timeSynchronization + "}}}";
}

//! Checks the defining property of the virtual time synchronization: a participant executes its step at 'now' only
//! if every other participant has completed all its steps that start before 'now'.
class CausalityChecker
{
public:
    void OnStepStarted(const std::string& name, std::chrono::nanoseconds now)
    {
        std::lock_guard<std::mutex> lock{_mx};
        for (const auto& other : _completedUntil)
        {
            if (other.first != name && other.second < now)
            {
                _violations.push_back(name + " started step at " + std::to_string(now.count()) + "ns while "
                                      + other.first + " only completed until "
                                      + std::to_string(other.second.count()) + "ns");
            }
        }
    }

    void OnStepCompleted(const std::string& name, std::chrono::nanoseconds now, std::chrono::nanoseconds duration)
    {
        std::lock_guard<std::mutex> lock{_mx};
        _completedUntil[name] = now + duration;
    }

    //! Participants that take part in the check from the start
    void Register(const std::string& name)
    {
        std::lock_guard<std::mutex> lock{_mx};
        _completedUntil.emplace(name, 0ns);
    }

    void Reset()
    {
        std::lock_guard<std::mutex> lock{_mx};
        _completedUntil.clear();
        _violations.clear();
    }

    auto Violations() -> std::vector<std::string>
    {
        std::lock_guard<std::mutex> lock{_mx};
        return _violations;
    }

private:
    std::mutex _mx;
    std::map<std::string, std::chrono::nanoseconds> _completedUntil;
    std::vector<std::string> _violations;
};

struct ITest_TimeCoordinator : ITest_SimTestHarness
{
    using ITest_SimTestHarness::ITest_SimTestHarness;

    void RunTestSetup(std::vector<ParticipantParams>& participantsParams, std::chrono::nanoseconds stopAt)
    {
        std::vector<std::string> participantNames;
        for (const auto& params : participantsParams)
        {
            participantNames.push_back(params.name);
            causality.Register(params.name);
        }
        SetupFromParticipantList(participantNames, /*enableInternalSystemMonitorTimeSync=*/false);

        std::mutex mx;
        for (auto& params : participantsParams)
        {
            auto&& simParticipant = _simTestHarness->GetParticipant(params.name, MakeParticipantConfiguration(params));
            auto* lifecycleService = simParticipant->GetOrCreateLifecycleService();
            auto* timeSyncService = simParticipant->GetOrCreateTimeSyncService();

            auto handler = [this, &params, &mx, lifecycleService, timeSyncService, stopAt](auto now, auto duration) {
                if (now >= stopAt)
                {
                    lifecycleService->Stop("stopping the test");
                    return;
                }

                causality.OnStepStarted(params.name, now);
                {
                    std::lock_guard<std::mutex> lock{mx};
                    params.steps.emplace_back(now, duration);
                }

                if (params.asyncStepHandler)
                {
                    std::thread{[this, &params, timeSyncService, now, duration] {
                        causality.OnStepCompleted(params.name, now, duration);
                        timeSyncService->CompleteSimulationStep();
                    }}.detach();
                }
                else
                {
                    causality.OnStepCompleted(params.name, now, duration);
                }
            };

            if (params.asyncStepHandler)
            {
                timeSyncService->SetSimulationStepHandlerAsync(handler, params.stepSize);
            }
            else
            {
                timeSyncService->SetSimulationStepHandler(handler, params.stepSize);
            }
        }

        ASSERT_TRUE(_simTestHarness->Run(10s)) << "SimTestHarness should terminate without timeout";
    }

    void ExpectContiguousSteps(const ParticipantParams& params, std::chrono::nanoseconds stopAt)
    {
        ASSERT_FALSE(params.steps.empty()) << params.name << " did not execute any step";
        auto expectedNow = 0ns;
        for (const auto& step : params.steps)
        {
            EXPECT_EQ(step.first, expectedNow) << params.name;
            expectedNow = step.first + step.second;
        }
        EXPECT_GE(expectedNow, stopAt) << params.name;
    }

    void ExpectNoCausalityViolations()
    {
        for (const auto& violation : causality.Violations())
        {
            ADD_FAILURE() << violation;
        }
    }

    CausalityChecker causality;
};

auto MixedStepSizeParticipants(std::optional<std::string> coordinatorName) -> std::vector<ParticipantParams>
{
    std::vector<ParticipantParams> participants{
        {"P1", 1ms}, {"P2", 2ms}, {"P3", 3ms}, {"P4", 5ms}, {"P5", 7ms},
    };
    for (auto& p : participants)
    {
        p.isTimeCoordinator = (coordinatorName == p.name);
    }
    return participants;
}

// The coordinated synchronization must execute exactly the same simulation steps as the distributed one
TEST_F(ITest_TimeCoordinator, coordinated_steps_match_distributed_steps)
{
    const auto stopAt = 50ms;

    auto distributed = MixedStepSizeParticipants(std::nullopt);
    RunTestSetup(distributed, stopAt);
    ExpectNoCausalityViolations();

    // Fresh checker and harness for the second run
    causality.Reset();

    auto coordinated = MixedStepSizeParticipants("P3");
    RunTestSetup(coordinated, stopAt);
    ExpectNoCausalityViolations();

    ASSERT_EQ(distributed.size(), coordinated.size());
    for (size_t i = 0; i < distributed.size(); ++i)
    {
        ExpectContiguousSteps(coordinated[i], stopAt);
        EXPECT_EQ(distributed[i].steps, coordinated[i].steps) << coordinated[i].name;
    }
}

TEST_F(ITest_TimeCoordinator, coordinator_with_async_step_handlers)
{
    const auto stopAt = 50ms;
    auto participants = MixedStepSizeParticipants("P1");
    participants[1].asyncStepHandler = true;
    participants[3].asyncStepHandler = true;
    participants[0].asyncStepHandler = true;

    RunTestSetup(participants, stopAt);
    ExpectNoCausalityViolations();
    for (const auto& p : participants)
    {
        ExpectContiguousSteps(p, stopAt);
    }
}

// Dynamic step sizes: every step is aligned to the minimal step among all participants, also when coordinated
TEST_F(ITest_TimeCoordinator, coordinator_with_dynamic_step_sizes)
{
    const auto stopAt = 30ms;
    std::vector<ParticipantParams> participants{{"P1", 1ms}, {"P2", 2ms}, {"P3", 3ms}};
    for (auto& p : participants)
    {
        p.dynamicSimulationStep = true;
    }
    participants[2].isTimeCoordinator = true;

    RunTestSetup(participants, stopAt);
    ExpectNoCausalityViolations();
    for (const auto& p : participants)
    {
        ExpectContiguousSteps(p, stopAt);
        for (const auto& step : p.steps)
        {
            EXPECT_EQ(step.second, 1ms) << p.name << " at " << step.first;
        }
    }
}

// An autonomous participant hops on into a coordinated simulation, runs some steps and hops off again
TEST_F(ITest_TimeCoordinator, autonomous_hop_on_hop_off)
{
    const auto stopAt = 200ms;
    const auto hopOnAt = 50ms;
    const auto hopOffAt = 100ms;
    const auto stepSize = 10ms;

    std::vector<ParticipantParams> participants{{"Coordinator", stepSize, true, true}, {"P2", stepSize}};
    for (auto& p : participants)
    {
        causality.Register(p.name);
    }
    SetupFromParticipantList({"Coordinator", "P2"}, /*enableInternalSystemMonitorTimeSync=*/false);

    std::mutex mx;
    std::condition_variable cv;
    Steps joinerSteps;
    bool joinerStepped{false};
    std::atomic<bool> joinerStarted{false};
    std::thread joinerThread;

    auto runJoiner = [&] {
        auto config = SilKit::Config::ParticipantConfigurationFromString(
            MakeParticipantConfiguration(ParticipantParams{"Joiner", stepSize}));
        auto participant = SilKit::CreateParticipant(config, "Joiner", _registryUri);
        auto* lifecycleService = participant->CreateLifecycleService({OperationMode::Autonomous});
        auto* timeSyncService = lifecycleService->CreateTimeSyncService();
        timeSyncService->SetSimulationStepHandler(
            [&, lifecycleService](auto now, auto duration) {
            if (now >= hopOffAt)
            {
                lifecycleService->Stop("hop off");
                return;
            }
            std::lock_guard<std::mutex> lock{mx};
            joinerSteps.emplace_back(now, duration);
            joinerStepped = true;
            cv.notify_all();
        }, stepSize);

        auto finalState = lifecycleService->StartLifecycle();
        EXPECT_EQ(finalState.wait_for(10s), std::future_status::ready);
    };

    for (auto& params : participants)
    {
        auto&& simParticipant = _simTestHarness->GetParticipant(params.name, MakeParticipantConfiguration(params));
        auto* lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto* timeSyncService = simParticipant->GetOrCreateTimeSyncService();

        auto handler = [&, lifecycleService, timeSyncService](auto now, auto duration) {
            if (now >= stopAt)
            {
                lifecycleService->Stop("stopping the test");
                return;
            }
            causality.OnStepStarted(params.name, now);
            {
                std::lock_guard<std::mutex> lock{mx};
                params.steps.emplace_back(now, duration);
            }

            if (!params.asyncStepHandler)
            {
                causality.OnStepCompleted(params.name, now, duration);
                return;
            }

            // The coordinator holds the time at hopOnAt until the joiner has executed its hop-on step
            std::thread{[&, now, duration, timeSyncService] {
                if (now == hopOnAt)
                {
                    joinerThread = std::thread{runJoiner};
                    joinerStarted = true;
                    std::unique_lock<std::mutex> lock{mx};
                    cv.wait_for(lock, 10s, [&] { return joinerStepped; });
                }
                causality.OnStepCompleted(params.name, now, duration);
                timeSyncService->CompleteSimulationStep();
            }}.detach();
        };

        if (params.asyncStepHandler)
        {
            timeSyncService->SetSimulationStepHandlerAsync(handler, params.stepSize);
        }
        else
        {
            timeSyncService->SetSimulationStepHandler(handler, params.stepSize);
        }
    }

    ASSERT_TRUE(_simTestHarness->Run(30s)) << "SimTestHarness should terminate without timeout";
    ASSERT_TRUE(joinerStarted);
    joinerThread.join();

    ExpectNoCausalityViolations();
    for (const auto& p : participants)
    {
        ExpectContiguousSteps(p, stopAt);
    }

    // The joiner started at the current simulation time and followed it in lockstep until it hopped off
    ASSERT_FALSE(joinerSteps.empty());
    EXPECT_EQ(joinerSteps.front().first, hopOnAt);
    auto expectedNow = hopOnAt;
    for (const auto& step : joinerSteps)
    {
        EXPECT_EQ(step.first, expectedNow);
        expectedNow += stepSize;
    }
    EXPECT_EQ(expectedNow, hopOffAt);
}

} // namespace
