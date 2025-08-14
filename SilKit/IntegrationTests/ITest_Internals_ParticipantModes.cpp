// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <queue>
#include <mutex>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/pubsub/PubSubSpec.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"

#include <execution>

namespace {

namespace SK {
using SilKit::IParticipant;
using SilKit::CreateParticipant;
using SilKit::Config::IParticipantConfiguration;
using SilKit::Config::ParticipantConfigurationFromString;
using SilKit::Experimental::Participant::CreateSystemController;
using SilKit::Experimental::Services::Orchestration::ISystemController;
using SilKit::Services::Orchestration::ILifecycleService;
using SilKit::Services::Orchestration::ITimeSyncService;
using SilKit::Services::Orchestration::ISystemMonitor;
using SilKit::Services::Orchestration::OperationMode;
using SilKit::Services::Orchestration::ParticipantState;
using SilKit::Services::Orchestration::ParticipantStatus;
using SilKit::Services::Orchestration::SystemState;
using SimulationStepHandler = SilKit::Services::Orchestration::ITimeSyncService::SimulationStepHandler;
using SystemStateHandler = SilKit::Services::Orchestration::ISystemMonitor::SystemStateHandler;
using ParticipantStatusHandler = SilKit::Services::Orchestration::ISystemMonitor::ParticipantStatusHandler;
using SilKit::Services::PubSub::IDataPublisher;
using SilKit::Services::PubSub::IDataSubscriber;
using SilKit::Services::PubSub::DataMessageEvent;
using SilKit::Services::PubSub::DataMessageHandler;
using SilKit::Services::PubSub::PubSubSpec;
using SilKit::Vendor::Vector::ISilKitRegistry;
using SilKit::Vendor::Vector::CreateSilKitRegistry;
using ByteSpan = SilKit::Util::Span<const std::uint8_t>;
} // namespace SK

using namespace std::chrono_literals;

constexpr auto SYSTEM_CONTROLLER_PARTICIPANT_NAME = "SystemController";
constexpr auto PUBSUB_TOPIC = "Topic";
constexpr auto PUBSUB_MEDIA_TYPE = "MediaType";

constexpr auto ASYNC_DELAY_BETWEEN_PUBLICATION = 50ms;

constexpr auto PARTICIPANT_CONFIGURATION = R"(
#Logging:
#  Sinks:
#    - Type: Stdout
#      Level: Debug

#Experimental:
#  TimeSynchronization:
#    AnimationFactor: 1.0
)";

enum class CoordinationMode
{
    Optional,
    Required,
};

enum struct ParticipantMode
{
    AutonomousFreerunning,
    AutonomousSynchronized,
    CoordinatedFreerunning,
    CoordinatedSynchronized,
    IgnorantFreerunning,
};

auto operator<<(std::ostream& os, const ParticipantMode runnerType) -> std::ostream&
{
    switch (runnerType)
    {
    case ParticipantMode::AutonomousFreerunning:
        os << "AutonomousFreerunning";
        break;
    case ParticipantMode::AutonomousSynchronized:
        os << "AutonomousSynchronized";
        break;
    case ParticipantMode::CoordinatedFreerunning:
        os << "CoordinatedFreerunning";
        break;
    case ParticipantMode::CoordinatedSynchronized:
        os << "CoordinatedSynchronized";
        break;
    case ParticipantMode::IgnorantFreerunning:
        os << "IgnorantFreerunning";
        break;
    }

    return os;
}

constexpr auto ALL_RUNNER_TYPES =
    std::array<ParticipantMode, 5>{ParticipantMode::AutonomousFreerunning, ParticipantMode::AutonomousSynchronized,
                                   ParticipantMode::CoordinatedFreerunning, ParticipantMode::CoordinatedSynchronized,
                                   ParticipantMode::IgnorantFreerunning};

// =====================================================================================================================

class WorkQueue
{
public:
    template <typename F>
    auto Push(F&& f) -> std::optional<std::future<void>>
    {
        if (_shutdown)
        {
            return std::nullopt;
        }

        std::packaged_task<void()> task{std::forward<F>(f)};
        auto future = task.get_future();

        {
            std::lock_guard<decltype(_mutex)> lock{_mutex};
            _queue.push(std::move(task));
        }

        _conditionVariable.notify_one();

        return future;
    }

    template <typename C, typename D>
    auto PullUntil(std::chrono::time_point<C, D> deadline) -> std::optional<std::packaged_task<void()>>
    {
        if (_shutdown)
        {
            return std::nullopt;
        }

        const auto predicate = [this] { return !_shutdown && !_queue.empty(); };

        std::packaged_task<void()> task;

        {
            std::unique_lock<decltype(_mutex)> lock{_mutex};

            if (!_conditionVariable.wait_until(lock, deadline, predicate))
            {
                return std::nullopt;
            }

            task = std::move(_queue.front());
            _queue.pop();
        }

        return task;
    }

    void ShutDown()
    {
        _shutdown = true;
        _conditionVariable.notify_all();
    }

private:
    std::mutex _mutex;
    std::condition_variable _conditionVariable;
    std::queue<std::packaged_task<void()>> _queue;

    std::atomic<bool> _shutdown = false;
};

// =====================================================================================================================

struct IRunner
{
    virtual ~IRunner() = default;

    /// Sets up the internals of the runner.
    virtual void SetUp() = 0;

    /// Runs the orchestrated runner.
    virtual void Main() = 0;

    /// Instructs the runner to stop 'normally'.
    virtual void TriggerHalt() = 0;

    /// Instructs the runner to abort it's 'main loop' regardless of, e.g., the participant lifecycle.
    virtual void TriggerAbort() = 0;
};

// =====================================================================================================================

struct IOrchestratorHandle
{
    virtual ~IOrchestratorHandle() = default;

    /// Create a participant with `name`.
    virtual auto CreateParticipant(std::string_view name) -> std::unique_ptr<SK::IParticipant> = 0;

    /// Notifies the orchestrator that `name` has received `message`.
    virtual void NotifyReceived(std::string_view name, SK::ByteSpan message) = 0;

    /// Notifies the orchestrator that `name` has changed its own participant state `participantState`.
    virtual void NotifyParticipantStateChanged(std::string_view name, SK::ParticipantState participantState) = 0;

    /// Notifies the orchestrator that `name` has received a system state change to `systemState`.
    virtual void NotifySystemStateChanged(std::string_view name, SK::SystemState systemState) = 0;

    /// Notifies the orchestrator that `name` has completed its main loop.
    virtual void NotifyMainLoopComplete(std::string_view name) = 0;

    /// Stop the simulation using 'normal' means.
    virtual void TriggerHalt() = 0;

    /// Stop the simulation 'as soon as possible', ignoring any participant's lifecycle.
    virtual void TriggerAbort() = 0;
};

// =====================================================================================================================

class RunnerBase
{
public:
    RunnerBase(IOrchestratorHandle& orchestrator, const std::string_view name)
        : _orchestrator{&orchestrator}
        , _name{name}
    {
    }

protected:
    void SetUpParticipant()
    {
        _participant = _orchestrator->CreateParticipant(_name);

        _systemMonitor = _participant->CreateSystemMonitor();
        _systemMonitor->AddSystemStateHandler(MakeSystemStateHandler());
        _systemMonitor->AddParticipantStatusHandler(MakeParticipantStatusHandler());
    }

    void SetUpCommunication()
    {
        _publisher = _participant->CreateDataPublisher("Publisher", MakeRunnerPubSubSpec(), 0);
        _subscriber =
            _participant->CreateDataSubscriber("Subscriber", MakeRunnerPubSubSpec(), MakeRunnerDataMessageHandler());
    }

    void SetUpLifecycle(SK::OperationMode operationMode)
    {
        _lifecycleService = _participant->CreateLifecycleService({operationMode});
    }

    void SetUpSynchronized()
    {
        _timeSyncService = _lifecycleService->CreateTimeSyncService();
        _timeSyncService->SetSimulationStepHandler(MakeSimulationStepHandler(), 1ms);
    }

protected:
    void MainFreerunning() const
    {
        auto finalParticipantState = _lifecycleService->StartLifecycle();

        while (!_aborted)
        {
            PublishMessage();

            const auto futureState = finalParticipantState.wait_for(ASYNC_DELAY_BETWEEN_PUBLICATION);

            if (futureState == std::future_status::deferred)
            {
                std::abort();
            }

            if (futureState == std::future_status::ready)
            {
                break;
            }
        }

        if (!_aborted)
        {
            finalParticipantState.get();
        }

        _orchestrator->NotifyMainLoopComplete(_name);
    }

    void MainSynchronized() const
    {
        auto finalParticipantState = _lifecycleService->StartLifecycle();

        while (!_aborted)
        {
            const auto futureState = finalParticipantState.wait_for(ASYNC_DELAY_BETWEEN_PUBLICATION);

            if (futureState == std::future_status::deferred)
            {
                std::abort();
            }

            if (futureState == std::future_status::ready)
            {
                break;
            }
        }

        if (!_aborted)
        {
            finalParticipantState.get();
        }

        _orchestrator->NotifyMainLoopComplete(_name);
    }

protected:
    void PublishMessage() const
    {
        auto message = std::vector<std::uint8_t>(_name.begin(), _name.end());

        _publisher->Publish(message);
    }

    void HaltLifecycle() const
    {
        _lifecycleService->Stop("Halt");
    }

    void AbortNow()
    {
        _aborted = true;
    }

private:
    [[nodiscard]] static auto MakeRunnerPubSubSpec() -> SK::PubSubSpec
    {
        return SK::PubSubSpec{PUBSUB_TOPIC, PUBSUB_MEDIA_TYPE};
    }

    [[nodiscard]] auto MakeRunnerDataMessageHandler() const -> SK::DataMessageHandler
    {
        return [this](auto, const SK::DataMessageEvent& event) { _orchestrator->NotifyReceived(_name, event.data); };
    }

    [[nodiscard]] auto MakeSystemStateHandler() const -> SK::SystemStateHandler
    {
        return [this](SK::SystemState systemState) { _orchestrator->NotifySystemStateChanged(_name, systemState); };
    }

    [[nodiscard]] auto MakeParticipantStatusHandler() const -> SK::ParticipantStatusHandler
    {
        return [this](const SK::ParticipantStatus& participantStatus) {
            if (participantStatus.participantName != _name)
            {
                return;
            }

            _orchestrator->NotifyParticipantStateChanged(_name, participantStatus.state);
        };
    }

    [[nodiscard]] auto MakeSimulationStepHandler() const -> SK::SimulationStepHandler
    {
        return [this](auto, auto) { PublishMessage(); };
    }

protected:
    // initialized in the constructor

    IOrchestratorHandle* _orchestrator;
    std::string _name;
    std::atomic<bool> _aborted = false;

    // initialized in SetUpParticipant

    std::unique_ptr<SK::IParticipant> _participant;
    SK::ISystemMonitor* _systemMonitor = nullptr;
    SK::IDataPublisher* _publisher = nullptr;
    SK::IDataSubscriber* _subscriber = nullptr;

    // initialized in SetUpLifecycle

    SK::ILifecycleService* _lifecycleService = nullptr;

    // initialized in SetUpSynchronized

    SK::ITimeSyncService* _timeSyncService = nullptr;
};

// =====================================================================================================================

class IgnorantFreerunning final
    : public IRunner
    , RunnerBase
{
    std::atomic<bool> _running = true;

public:
    using RunnerBase::RunnerBase;

private: // IRunner
    void SetUp() override
    {
        SetUpParticipant();
        SetUpCommunication();
    }

    void Main() override
    {
        while (_running && !_aborted)
        {
            PublishMessage();
            std::this_thread::sleep_for(ASYNC_DELAY_BETWEEN_PUBLICATION);
        }

        _orchestrator->NotifyMainLoopComplete(_name);
    }

    void TriggerHalt() override
    {
        _running = false;
    }

    void TriggerAbort() override
    {
        AbortNow();
    }
};

class CoordinatedSynchronized final
    : public IRunner
    , RunnerBase
{
public:
    using RunnerBase::RunnerBase;

private: // IRunner
    void SetUp() override
    {
        SetUpParticipant();
        SetUpCommunication();
        SetUpLifecycle(SK::OperationMode::Coordinated);
        SetUpSynchronized();
    }

    void Main() override
    {
        MainSynchronized();
    }

    void TriggerHalt() override
    {
        HaltLifecycle();
    }

    void TriggerAbort() override
    {
        AbortNow();
    }
};

class CoordinatedFreerunning final
    : public IRunner
    , RunnerBase
{
public:
    using RunnerBase::RunnerBase;

private: // IRunner
    void SetUp() override
    {
        SetUpParticipant();
        SetUpCommunication();
        SetUpLifecycle(SK::OperationMode::Coordinated);
    }

    void Main() override
    {
        MainFreerunning();
    }

    void TriggerHalt() override
    {
        HaltLifecycle();
    }

    void TriggerAbort() override
    {
        AbortNow();
    }
};

class AutonomousSynchronized final
    : public IRunner
    , RunnerBase
{
public:
    using RunnerBase::RunnerBase;

private: // IRunner
    void SetUp() override
    {
        SetUpParticipant();
        SetUpCommunication();
        SetUpLifecycle(SK::OperationMode::Autonomous);
        SetUpSynchronized();
    }

    void Main() override
    {
        MainSynchronized();
    }

    void TriggerHalt() override
    {
        HaltLifecycle();
    }

    void TriggerAbort() override
    {
        AbortNow();
    }
};

class AutonomousFreerunning final
    : public IRunner
    , RunnerBase
{
public:
    using RunnerBase::RunnerBase;

private: // IRunner
    void SetUp() override
    {
        SetUpParticipant();
        SetUpCommunication();
        SetUpLifecycle(SK::OperationMode::Autonomous);
    }

    void Main() override
    {
        MainFreerunning();
    }

    void TriggerHalt() override
    {
        HaltLifecycle();
    }

    void TriggerAbort() override
    {
        AbortNow();
    }
};

class SystemController final
    : public IRunner
    , RunnerBase
{
    // initialized in the constructor

    std::vector<std::string> _requiredParticipantNames;
    std::atomic<bool> _running = true;

    // initialized in SetUp

    SK::ISystemController* _systemController = nullptr;

public:
    SystemController(IOrchestratorHandle& orchestrator, std::vector<std::string> requiredParticipantNames)
        : RunnerBase(orchestrator, SYSTEM_CONTROLLER_PARTICIPANT_NAME)
        , _requiredParticipantNames{std::move(requiredParticipantNames)}
    {
    }

private: // IRunner
    void SetUp() override
    {
        SetUpParticipant();

        _systemController = SK::CreateSystemController(_participant.get());
        _systemController->SetWorkflowConfiguration({_requiredParticipantNames});
    }

    void Main() override
    {
        while (_running && !_aborted)
        {
            std::this_thread::sleep_for(ASYNC_DELAY_BETWEEN_PUBLICATION);
        }
    }

    void TriggerHalt() override
    {
        _running = false;
    }

    void TriggerAbort() override
    {
        AbortNow();
    }
};

// =====================================================================================================================

class Orchestrator final : public IOrchestratorHandle
{
public:
    struct RunResult
    {
        bool communicatedWithEveryone = false;
        bool seenAnyParticipantStateError = false;
        bool enteredSystemStateError = false;
        bool aborted = false;
    };

private:
    struct ParticipantRunner
    {
        std::unique_ptr<IRunner> runner;
        std::string name;
        CoordinationMode coordinationMode;
    };

public:
    void StartRegistry()
    {
        _registry = MakeRegistry();
        _registryUri = _registry->StartListening("silkit://127.0.0.1:0");
    }

    template <typename T>
    void AddOptional(std::string_view name)
    {
        AddParticipantRunner({
            std::make_unique<T>(*this, name),
            std::string{name},
            CoordinationMode::Optional,
        });
    }

    template <typename T>
    void AddRequired(std::string_view name)
    {
        AddParticipantRunner(ParticipantRunner{
            std::make_unique<T>(*this, name),
            std::string{name},
            CoordinationMode::Required,
        });
    }

    void AddAutomatic(const ParticipantMode mode, std::string_view name)
    {
        switch (mode)
        {
        case ParticipantMode::AutonomousFreerunning:
            AddOptional<AutonomousFreerunning>(name);
            break;
        case ParticipantMode::AutonomousSynchronized:
            AddOptional<AutonomousSynchronized>(name);
            break;
        case ParticipantMode::CoordinatedFreerunning:
            AddRequired<CoordinatedFreerunning>(name);
            break;
        case ParticipantMode::CoordinatedSynchronized:
            AddRequired<CoordinatedSynchronized>(name);
            break;
        case ParticipantMode::IgnorantFreerunning:
            AddOptional<IgnorantFreerunning>(name);
        }
    }

    auto Run() -> RunResult
    {
        CreateAndAddSystemControllerRunnerIfNecessary();

        for (const auto& runner : _runners)
        {
            auto future = std::async(std::launch::async, [self = runner] {
                self->SetUp();
                self->Main();
            });

            _runnerFutures.emplace_back(std::move(future));
        }

        WorkFor(30s);

        WaitForAllRunnersDoneOrCrash(10s);

        for (auto& future : _runnerFutures)
        {
            future.get();
        }

        return _runResult;
    }

private:
    void WorkFor(const std::chrono::nanoseconds duration)
    {
        const auto deadline = std::chrono::steady_clock::now() + duration;

        while (true)
        {
            auto task = _workQueue.PullUntil(deadline);

            if (!task.has_value())
            {
                return;
            }

            (*task)();
        }
    }

    /// Returns the number of runners that have not stopped yet.
    auto CountActiveRunners() const -> std::size_t
    {
        std::size_t count = 0;

        for (auto& future : _runnerFutures)
        {
            count += future.wait_for(1ns) != std::future_status::ready;
        }

        return count;
    }

    /// Waits `duration` for all runners to stop. Crashes the process if any runner does not exit in time.
    void WaitForAllRunnersDoneOrCrash(const std::chrono::nanoseconds duration) const
    {
        const auto deadline = std::chrono::steady_clock::now() + duration;

        std::size_t count;

        do
        {
            count = CountActiveRunners();

            if (count == 0)
            {
                return;
            }
        } while (std::chrono::steady_clock::now() < deadline);

        // if any runner refuses to stop, we can't do anything sensible but crash the process
        std::cerr << "FATAL ERROR: " << count << " runners are refusing to stop" << std::endl;
        std::abort();
    }

private:
    void AddParticipantRunner(ParticipantRunner participantRunner)
    {
        auto runner = participantRunner.runner.get();

        _participantRunners.emplace_back(std::move(participantRunner));
        _runners.emplace_back(runner);
    }

    void CreateAndAddSystemControllerRunnerIfNecessary()
    {
        const auto requiredParticipantNames = CollectRequiredParticipantNames();

        if (requiredParticipantNames.empty())
        {
            std::cerr << "No system controller necessary, as there are no required participants" << std::endl;
            return;
        }

        _systemControllerRunner = std::make_unique<SystemController>(*this, requiredParticipantNames);
        _runners.emplace_back(_systemControllerRunner.get());
    }

    [[nodiscard]] auto CollectRequiredParticipantNames() const -> std::vector<std::string>
    {
        std::vector<std::string> requiredParticipantNames;

        for (const auto& runner : _participantRunners)
        {
            if (runner.coordinationMode == CoordinationMode::Required)
            {
                requiredParticipantNames.push_back(runner.name);
            }
        }

        return requiredParticipantNames;
    }

private: // IOrchestratorHandle
    auto CreateParticipant(std::string_view name) -> std::unique_ptr<SK::IParticipant> override
    {
        return MakeParticipant(std::string{name}, _registryUri);
    }

    void NotifyReceived(std::string_view name, SK::ByteSpan message) override
    {
        auto work = [this, name = std::string{name}, message = ToStdVector(message)]() mutable {
            return HandleNotifyReceived(name, std::move(message));
        };

        _workQueue.Push(std::move(work));
    }

    void NotifySystemStateChanged(std::string_view name, SK::SystemState systemState) override
    {
        auto work = [this, name = std::string{name}, systemState] {
            return HandleNotifySystemStateChanged(name, systemState);
        };

        _workQueue.Push(std::move(work));
    }

    void NotifyParticipantStateChanged(std::string_view name, SK::ParticipantState participantState) override
    {
        auto work = [this, name = std::string{name}, participantState] {
            return HandleNotifyParticipantStateChanged(name, participantState);
        };

        _workQueue.Push(std::move(work));
    }

    void NotifyMainLoopComplete(std::string_view name) override
    {
        auto work = [this, name = std::string{name}] { return HandleNotifyMainLoopComplete(name); };

        _workQueue.Push(std::move(work));
    }

    void TriggerHalt() override
    {
        _workQueue.Push([this] { return HandleTriggerHalt(); });
    }

    void TriggerAbort() override
    {
        _workQueue.Push([this] { return HandleTriggerAbort(); });
    }

private:
    void HandleNotifyReceived(const std::string& name, std::vector<std::uint8_t> message)
    {
        std::string other(message.begin(), message.end());

        _received[name].insert(other);

        _runResult.communicatedWithEveryone = std::all_of(
            _received.begin(), _received.end(), [expectedSize = _participantRunners.size()](const auto& item) {
            const auto& [_, senders] = item;
            return senders.size() == expectedSize;
        });

        if (_runResult.communicatedWithEveryone)
        {
            std::cerr
                << "Halting because each participant has received at least one message from every other participant"
                << std::endl;
            TriggerHalt();
        }
    }

    void HandleNotifyParticipantStateChanged(const std::string& name, SK::ParticipantState participantState)
    {
        std::cerr << std::quoted(name) << " reported participant state " << participantState << std::endl;

        switch (participantState)
        {
        case SK::ParticipantState::Error:
            _runResult.seenAnyParticipantStateError = true;
            TriggerAbort();
            break;
        default:
            break;
        }
    }

    void HandleNotifySystemStateChanged(const std::string& name, SK::SystemState systemState)
    {
        std::cerr << std::quoted(name) << " reported system state " << systemState << std::endl;

        switch (systemState)
        {
        case SK::SystemState::Error:
            _runResult.enteredSystemStateError = true;
            TriggerAbort();
            break;
        case SK::SystemState::Shutdown:
            TriggerHalt();
            break;
        default:
            break;
        }
    }

    void HandleNotifyMainLoopComplete(const std::string& name)
    {
        _mainLoopCompleted.emplace(name);

        if (_mainLoopCompleted.size() == _participantRunners.size())
        {
            std::cerr << "All runners have completed their main loop, shutting down work queue" << std::endl;
            _workQueue.ShutDown();
        }
    }

    void HandleTriggerHalt() const
    {
        std::cerr << "DoHalt()" << std::endl;

        for (const auto& runner : _runners)
        {
            runner->TriggerHalt();
        }
    }

    void HandleTriggerAbort()
    {
        std::cerr << "DoAbort()" << std::endl;

        _runResult.aborted = true;

        for (const auto& runner : _runners)
        {
            runner->TriggerAbort();
        }

        _workQueue.ShutDown();
    }

private:
    static auto MakeRegistry() -> std::unique_ptr<SK::ISilKitRegistry>
    {
        auto config = SK::ParticipantConfigurationFromString(PARTICIPANT_CONFIGURATION);
        return SK::CreateSilKitRegistry(config);
    }

    static auto MakeParticipant(const std::string& name,
                                const std::string& registryUri) -> std::unique_ptr<SK::IParticipant>
    {
        const auto config = SK::ParticipantConfigurationFromString(PARTICIPANT_CONFIGURATION);
        return SK::CreateParticipant(config, name, registryUri);
    }

private:
    std::unique_ptr<SK::ISilKitRegistry> _registry;
    std::string _registryUri;

    std::vector<ParticipantRunner> _participantRunners;
    std::unique_ptr<IRunner> _systemControllerRunner;

    std::vector<IRunner*> _runners;
    std::vector<std::future<void>> _runnerFutures;

    std::unordered_map<std::string, std::set<std::string>> _received;
    std::unordered_set<std::string> _mainLoopCompleted;

    RunResult _runResult;

    WorkQueue _workQueue;
};

// =====================================================================================================================

TEST(ITest_ParticipantModes, ValidCombinationsCommunicateAndStopCleanly)
{
    std::vector<std::pair<ParticipantMode, ParticipantMode>> combinations;

    for (const auto a : ALL_RUNNER_TYPES)
    {
        for (const auto b : ALL_RUNNER_TYPES)
        {
            if (a == ParticipantMode::CoordinatedSynchronized && b == ParticipantMode::AutonomousSynchronized)
            {
                continue;
            }

            if (a == ParticipantMode::AutonomousSynchronized && b == ParticipantMode::CoordinatedSynchronized)
            {
                continue;
            }

            combinations.emplace_back(a, b);
        }
    }

    for (const auto& [a, b] : combinations)
    {
        std::cerr << "Combination: " << a << " + " << b << std::endl;

        Orchestrator orchestrator;

        orchestrator.StartRegistry();

        orchestrator.AddAutomatic(a, "A1");
        orchestrator.AddAutomatic(a, "A2");
        orchestrator.AddAutomatic(a, "A3");
        orchestrator.AddAutomatic(a, "A4");

        orchestrator.AddAutomatic(b, "B1");
        orchestrator.AddAutomatic(b, "B2");
        orchestrator.AddAutomatic(b, "B3");
        orchestrator.AddAutomatic(b, "B4");

        const auto result = orchestrator.Run();

        ASSERT_TRUE(result.communicatedWithEveryone);
        ASSERT_FALSE(result.seenAnyParticipantStateError);
        ASSERT_FALSE(result.enteredSystemStateError);
        ASSERT_FALSE(result.aborted);
    }
}

TEST(ITest_ParticipantModes, OptionalIgnorantFreerunning)
{
    Orchestrator orchestrator;

    orchestrator.StartRegistry();

    orchestrator.AddOptional<IgnorantFreerunning>("O-IF-1");
    orchestrator.AddOptional<IgnorantFreerunning>("O-IF-2");
    orchestrator.AddOptional<IgnorantFreerunning>("O-IF-3");
    orchestrator.AddOptional<IgnorantFreerunning>("O-IF-4");

    const auto result = orchestrator.Run();

    ASSERT_TRUE(result.communicatedWithEveryone);
    ASSERT_FALSE(result.seenAnyParticipantStateError);
    ASSERT_FALSE(result.enteredSystemStateError);
    ASSERT_FALSE(result.aborted);
}

TEST(ITest_ParticipantModes, OptionalAutonomousFreerunning)
{
    Orchestrator orchestrator;

    orchestrator.StartRegistry();

    orchestrator.AddOptional<AutonomousFreerunning>("O-AF-1");
    orchestrator.AddOptional<AutonomousFreerunning>("O-AF-2");
    orchestrator.AddOptional<AutonomousFreerunning>("O-AF-3");
    orchestrator.AddOptional<AutonomousFreerunning>("O-AF-4");

    const auto result = orchestrator.Run();

    ASSERT_TRUE(result.communicatedWithEveryone);
    ASSERT_FALSE(result.seenAnyParticipantStateError);
    ASSERT_FALSE(result.enteredSystemStateError);
    ASSERT_FALSE(result.aborted);
}

TEST(ITest_ParticipantModes, OptionalAutonomousSynchronized)
{
    Orchestrator orchestrator;

    orchestrator.StartRegistry();

    orchestrator.AddOptional<AutonomousSynchronized>("O-AS-1");
    orchestrator.AddOptional<AutonomousSynchronized>("O-AS-2");
    orchestrator.AddOptional<AutonomousSynchronized>("O-AS-3");
    orchestrator.AddOptional<AutonomousSynchronized>("O-AS-4");

    const auto result = orchestrator.Run();

    ASSERT_TRUE(result.communicatedWithEveryone);
    ASSERT_FALSE(result.seenAnyParticipantStateError);
    ASSERT_FALSE(result.enteredSystemStateError);
    ASSERT_FALSE(result.aborted);
}

TEST(ITest_ParticipantModes, RequiredIgnorantFreerunning)
{
    Orchestrator orchestrator;

    orchestrator.StartRegistry();

    orchestrator.AddRequired<IgnorantFreerunning>("R-IF-1");
    orchestrator.AddRequired<IgnorantFreerunning>("R-IF-2");
    orchestrator.AddRequired<IgnorantFreerunning>("R-IF-3");
    orchestrator.AddRequired<IgnorantFreerunning>("R-IF-4");

    const auto result = orchestrator.Run();

    ASSERT_TRUE(result.communicatedWithEveryone);
    ASSERT_FALSE(result.seenAnyParticipantStateError);
    ASSERT_FALSE(result.enteredSystemStateError);
    ASSERT_FALSE(result.aborted);
}

TEST(ITest_ParticipantModes, RequiredAutonomousFreerunning)
{
    Orchestrator orchestrator;

    orchestrator.StartRegistry();

    orchestrator.AddRequired<AutonomousFreerunning>("R-AF-1");
    orchestrator.AddRequired<AutonomousFreerunning>("R-AF-2");
    orchestrator.AddRequired<AutonomousFreerunning>("R-AF-3");
    orchestrator.AddRequired<AutonomousFreerunning>("R-AF-4");

    const auto result = orchestrator.Run();

    ASSERT_TRUE(result.communicatedWithEveryone);
    ASSERT_FALSE(result.seenAnyParticipantStateError);
    ASSERT_FALSE(result.enteredSystemStateError);
    ASSERT_FALSE(result.aborted);
}

TEST(ITest_ParticipantModes, RequiredAutonomousSynchronized)
{
    Orchestrator orchestrator;

    orchestrator.StartRegistry();

    orchestrator.AddRequired<AutonomousSynchronized>("R-AS-1");
    orchestrator.AddRequired<AutonomousSynchronized>("R-AS-2");
    orchestrator.AddRequired<AutonomousSynchronized>("R-AS-3");
    orchestrator.AddRequired<AutonomousSynchronized>("R-AS-4");

    const auto result = orchestrator.Run();

    ASSERT_TRUE(result.communicatedWithEveryone);
    ASSERT_FALSE(result.seenAnyParticipantStateError);
    ASSERT_FALSE(result.enteredSystemStateError);
    ASSERT_FALSE(result.aborted);
}

TEST(ITest_ParticipantModes, RequiredCoordinatedFreerunning)
{
    Orchestrator orchestrator;

    orchestrator.StartRegistry();

    orchestrator.AddRequired<CoordinatedFreerunning>("R-CF-1");
    orchestrator.AddRequired<CoordinatedFreerunning>("R-CF-2");
    orchestrator.AddRequired<CoordinatedFreerunning>("R-CF-3");
    orchestrator.AddRequired<CoordinatedFreerunning>("R-CF-4");

    const auto result = orchestrator.Run();

    ASSERT_TRUE(result.communicatedWithEveryone);
    ASSERT_FALSE(result.seenAnyParticipantStateError);
    ASSERT_FALSE(result.enteredSystemStateError);
    ASSERT_FALSE(result.aborted);
}

TEST(ITest_ParticipantModes, RequiredCoordinatedSynchronized)
{
    Orchestrator orchestrator;

    orchestrator.StartRegistry();

    orchestrator.AddRequired<CoordinatedSynchronized>("R-CS-1");
    orchestrator.AddRequired<CoordinatedSynchronized>("R-CS-2");
    orchestrator.AddRequired<CoordinatedSynchronized>("R-CS-3");
    orchestrator.AddRequired<CoordinatedSynchronized>("R-CS-4");

    const auto result = orchestrator.Run();

    ASSERT_TRUE(result.communicatedWithEveryone);
    ASSERT_FALSE(result.seenAnyParticipantStateError);
    ASSERT_FALSE(result.enteredSystemStateError);
    ASSERT_FALSE(result.aborted);
}

TEST(ITest_ParticipantModes, AutonomousFreerunningRequiredWorksWithCoordinatedSynchronized)
{
    Orchestrator orchestrator;

    orchestrator.StartRegistry();

    orchestrator.AddRequired<AutonomousFreerunning>("R-AF-1");
    orchestrator.AddRequired<AutonomousFreerunning>("R-AF-2");
    orchestrator.AddRequired<AutonomousFreerunning>("R-AF-3");
    orchestrator.AddRequired<AutonomousFreerunning>("R-AF-4");

    orchestrator.AddRequired<CoordinatedSynchronized>("R-CS-1");
    orchestrator.AddRequired<CoordinatedSynchronized>("R-CS-2");
    orchestrator.AddRequired<CoordinatedSynchronized>("R-CS-3");
    orchestrator.AddRequired<CoordinatedSynchronized>("R-CS-4");

    const auto result = orchestrator.Run();

    ASSERT_TRUE(result.communicatedWithEveryone);
    ASSERT_FALSE(result.seenAnyParticipantStateError);
    ASSERT_FALSE(result.enteredSystemStateError);
    ASSERT_FALSE(result.aborted);
}

TEST(ITest_ParticipantModes, OptionalCoordinatedFreerunningFails)
{
    Orchestrator orchestrator;

    orchestrator.StartRegistry();

    orchestrator.AddOptional<CoordinatedFreerunning>("O-CF-1");

    // required dummy participant such that the system controller is started
    orchestrator.AddRequired<IgnorantFreerunning>("R-IF-1");

    const auto result = orchestrator.Run();

    ASSERT_TRUE(result.seenAnyParticipantStateError);
    ASSERT_TRUE(result.aborted);
}

TEST(ITest_ParticipantModes, OptionalCoordinatedSynchronizedFails)
{
    Orchestrator orchestrator;

    orchestrator.StartRegistry();

    orchestrator.AddOptional<CoordinatedSynchronized>("O-CS-1");

    // required dummy participant such that the system controller is started
    orchestrator.AddRequired<IgnorantFreerunning>("R-IF-1");

    const auto result = orchestrator.Run();

    ASSERT_TRUE(result.seenAnyParticipantStateError);
    ASSERT_TRUE(result.aborted);
}

TEST(ITest_ParticipantModes, CreateLifecycleServiceThrowsWhenOperationModeIsInvalid)
{
    const auto participantConfiguration = SK::ParticipantConfigurationFromString(PARTICIPANT_CONFIGURATION);

    const auto registry = SK::CreateSilKitRegistry(participantConfiguration);
    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");

    const auto participant = SK::CreateParticipant(participantConfiguration, "P", registryUri);

    ASSERT_THROW(participant->CreateLifecycleService({SK::OperationMode::Invalid}), SilKit::ConfigurationError);
}

} // anonymous namespace
