/* Copyright (c) 2023 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <cstdlib>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <list>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"

#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/pubsub/PubSubSpec.hpp"

#include "ConfigurationTestUtils.hpp"
#include "functional.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"

using namespace std::chrono_literals;
using namespace SilKit;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Config;
using namespace SilKit::Services::PubSub;
using namespace testing;

const std::string systemControllerParticipantName{"systemControllerParticipant"};
const std::string topic{"Topic"};
const std::string mediaType{"A"};
static size_t expectedReceptions;
static const std::chrono::milliseconds communicationTimeout{8000ms};
static const std::chrono::milliseconds asyncDelayBetweenPublication{50ms};
static const std::chrono::seconds simStepSize{1s};

static size_t globalParticipantIndex = 0;
static std::vector<std::string> participantNames{};
static std::map<size_t, bool> participantIsSync{};

const bool verbose = true;
const bool logging = false;
const Services::Logging::Level logLevel = Services::Logging::Level::Debug;

enum class TimeMode
{
    Async,
    Sync
};

class ITest_MultiThreadedParticipants : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD(void, AbortHandler, (ParticipantState));
        MOCK_METHOD(void, ParticipantStateHandler, (ParticipantState)); // helper
    };

    struct TestParticipant
    {
        TestParticipant(const std::string& newName, TimeMode newTimeMode, OperationMode mode)
        {
            participantNames.push_back(newName);
            participantIsSync.emplace(globalParticipantIndex, newTimeMode == TimeMode::Sync);

            name = newName;
            id = static_cast<uint8_t>(globalParticipantIndex++);
            timeMode = newTimeMode;
            lifeCycleOperationMode = mode;
        }

        TestParticipant(TestParticipant&&) = default;
        TestParticipant& operator=(TestParticipant&&) = default;

        struct ImmovableMembers
        {
            ImmovableMembers() = default;

            ImmovableMembers(ImmovableMembers&& other) noexcept
                : allReceived{other.allReceived.load()}
                , _runAsync{other._runAsync.load()}
                , stopRequested{other.stopRequested.load()}
                , runningStateReached{other.runningStateReached.load()}
                , pausedStateReached{other.pausedStateReached.load()}
                , communicationInitializedStateReached{other.communicationInitializedStateReached.load()}
            {
            }

            ImmovableMembers& operator=(ImmovableMembers&& other) noexcept
            {
                if (this != &other)
                {
                    allReceived = other.allReceived.load();
                    _runAsync = other._runAsync.load();
                    stopRequested = other.stopRequested.load();
                    runningStateReached = other.runningStateReached.load();
                    pausedStateReached = other.pausedStateReached.load();
                    communicationInitializedStateReached = other.communicationInitializedStateReached.load();
                }

                return *this;
            }

            mutable std::mutex mutex;
            std::atomic<bool> allReceived{false};
            std::atomic<bool> _runAsync{true};
            std::atomic<bool> stopRequested{false};
            std::atomic<bool> runningStateReached{false};
            std::atomic<bool> pausedStateReached{false};
            std::atomic<bool> communicationInitializedStateReached{false};
        };

        ImmovableMembers i{};

        std::string name;
        uint8_t id;
        std::unique_ptr<IParticipant> participant;
        IDataPublisher* publisher;
        IDataSubscriber* subscriber;
        std::set<uint8_t> receivedIds;
        std::promise<void> allReceivedPromise;

        struct PubData
        {
            uint8_t participantId;
            int64_t nowNs;
        };

        std::chrono::nanoseconds now{-1};
        bool simtimePassed{false};
        std::promise<void> simtimePassedPromise;

        std::promise<void> runningStatePromise;
        std::promise<void> pausedStatePromise;
        std::promise<void> communicationInitializedStatePromise;
        TimeMode timeMode;
        OperationMode lifeCycleOperationMode;
        ILifecycleService* lifecycleService{nullptr};

        std::map<std::string, ParticipantState> participantStates;

        void ResetReception()
        {
            receivedIds.clear();
            allReceivedPromise = std::promise<void>{};
            i.allReceived = false;
        }

        void AwaitCommunication()
        {
            auto futureStatus = allReceivedPromise.get_future().wait_for(communicationTimeout);
            if (futureStatus != std::future_status::ready)
            {
                FAIL() << "Test Failure: Awaiting test communication timed out";
            }
            else if (verbose)
            {
                std::cout << ">> AwaitCommunication of " << name << " done" << std ::endl;
            }
        }

        void AwaitRunning()
        {
            auto runningStateFuture = runningStatePromise.get_future();
            auto futureStatus = runningStateFuture.wait_for(communicationTimeout);
            if (futureStatus != std::future_status::ready)
            {
                FAIL() << "Test Failure: Awaiting system controller timed out";
            }
            else if (verbose)
            {
                std::cout << ">> AwaitRunning of " << name << " done" << std ::endl;
            }
        }

        void AwaitPaused()
        {
            auto pausedStateFuture = pausedStatePromise.get_future();
            auto futureStatus = pausedStateFuture.wait_for(communicationTimeout);
            if (futureStatus != std::future_status::ready)
            {
                FAIL() << "Test Failure: Awaiting " << name << " timed out";
            }
            else if (verbose)
            {
                std::cout << ">> AwaitPaused of " << name << " done" << std ::endl;
            }
        }
        void AwaitCommunicationInitialized()
        {
            auto communicationInitializedStateFuture = communicationInitializedStatePromise.get_future();
            auto futureStatus = communicationInitializedStateFuture.wait_for(communicationTimeout);
            if (futureStatus != std::future_status::ready)
            {
                FAIL() << "Test Failure: Awaiting " << name << " timed out";
            }
            else if (verbose)
            {
                std::cout << ">> AwaitCommunicationInitialized of " << name << " done" << std ::endl;
            }
        }

        void SetMonitoredParticipantState(const std::string& participantName, ParticipantState participantState)
        {
            std::lock_guard<decltype(i.mutex)> lock{i.mutex};
            participantStates[participantName] = participantState;
        }

        auto CopyMonitoredParticipantStates() const -> std::map<std::string, ParticipantState>
        {
            std::lock_guard<decltype(i.mutex)> lock{i.mutex};
            return participantStates;
        }
    };

    struct SystemControllerParticipant
    {
        struct ImmovableMembers
        {
            ImmovableMembers() = default;
            ImmovableMembers(ImmovableMembers&& other) noexcept
                : runningStateReached{other.runningStateReached.load()}
                , pausedStateReached{other.pausedStateReached.load()}
                , communicationInitializedStateReached{other.communicationInitializedStateReached.load()}
            {
            }

            ImmovableMembers& operator=(ImmovableMembers&& other) noexcept
            {
                if (this != &other)
                {
                    runningStateReached = other.runningStateReached.load();
                    pausedStateReached = other.pausedStateReached.load();
                    communicationInitializedStateReached = other.communicationInitializedStateReached.load();
                }

                return *this;
            }

            std::atomic<bool> runningStateReached{false};
            std::atomic<bool> pausedStateReached{false};
            std::atomic<bool> communicationInitializedStateReached{false};
        };

        ImmovableMembers i{};
        std::promise<void> runningStatePromise;
        std::promise<void> pausedStatePromise;
        std::promise<void> communicationInitializedStatePromise;
        std::unique_ptr<IParticipant> participant;
        SilKit::Experimental::Services::Orchestration::ISystemController* systemController;
        ISystemMonitor* systemMonitor;
        ILifecycleService* lifecycleService;
        std::future<ParticipantState> finalState;

        void AwaitRunning()
        {
            auto runningStateFuture = runningStatePromise.get_future();
            auto futureStatus = runningStateFuture.wait_for(communicationTimeout);
            if (futureStatus != std::future_status::ready)
            {
                FAIL() << "Test Failure: Awaiting system controller timed out";
            }
            else if (verbose)
            {
                std::cout << ">> AwaitRunning of " << systemControllerParticipantName << " done" << std ::endl;
            }
        }

        void AwaitPaused()
        {
            auto pausedStateFuture = pausedStatePromise.get_future();
            auto futureStatus = pausedStateFuture.wait_for(communicationTimeout);
            if (futureStatus != std::future_status::ready)
            {
                FAIL() << "Test Failure: Awaiting system controller timed out";
            }
            else if (verbose)
            {
                std::cout << ">> AwaitPaused of " << systemControllerParticipantName << " done" << std ::endl;
            }
        }

        void AwaitCommunicationInitialized()
        {
            auto futureStatus = communicationInitializedStatePromise.get_future().wait_for(communicationTimeout);
            if (futureStatus != std::future_status::ready)
            {
                FAIL() << "Test Failure: Awaiting test communication timed out";
            }
            else if (verbose)
            {
                std::cout << ">> AwaitCommunicationInitialized of " << systemControllerParticipantName << " done"
                          << std ::endl;
            }
        }
    };

    void SystemStateHandler(SystemState newState)
    {
        switch (newState)
        {
        case SystemState::Error:
            if (verbose)
            {
                std::cout << "SystemState::Error -> Aborting simulation" << std ::endl;
            }
            systemControllerParticipant.systemController->AbortSimulation();
            break;

        case SystemState::Running:
            break;

        default:
            break;
        }
    }

    void SyncParticipantThread(TestParticipant& testParticipant)
    {
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> config{nullptr};
        if (logging)
        {
            config = SilKit::Config::MakeParticipantConfigurationWithLogging(logLevel);
        }
        else
        {
            config = SilKit::Config::MakeEmptyParticipantConfiguration();
        }

        testParticipant.participant = SilKit::CreateParticipant(config, testParticipant.name, _registryUri);
        testParticipant.lifecycleService =
            testParticipant.participant->CreateLifecycleService({testParticipant.lifeCycleOperationMode});
        testParticipant.lifecycleService->SetAbortHandler(
            SilKit::Util::bind_method(&callbacks, &Callbacks::AbortHandler));

        auto systemMonitor = testParticipant.participant->CreateSystemMonitor();
        systemMonitor->AddParticipantStatusHandler([&testParticipant](ParticipantStatus status) {
            if (status.participantName == testParticipant.name && status.state == ParticipantState::Paused
                && !testParticipant.i.pausedStateReached)
            {
                testParticipant.i.pausedStateReached = true;
                testParticipant.pausedStatePromise.set_value();
            }
            if (status.participantName == testParticipant.name && status.state == ParticipantState::Running
                && !testParticipant.i.runningStateReached)
            {
                testParticipant.i.runningStateReached = true;
                testParticipant.runningStatePromise.set_value();
            }
        });

        auto* timeSyncService = testParticipant.lifecycleService->CreateTimeSyncService();

        SilKit::Services::PubSub::PubSubSpec dataSpec{topic, mediaType};
        SilKit::Services::PubSub::PubSubSpec matchingDataSpec{topic, mediaType};
        testParticipant.publisher = testParticipant.participant->CreateDataPublisher("TestPublisher", dataSpec, 0);
        testParticipant.subscriber = testParticipant.participant->CreateDataSubscriber(
            "TestSubscriber", matchingDataSpec,
            [timeSyncService, &testParticipant](IDataSubscriber* /*subscriber*/,
                                                const DataMessageEvent& dataMessageEvent) {
            auto participantId = dataMessageEvent.data[0];

            if (testParticipant.i.allReceived || participantId == testParticipant.id)
                return;

            auto now = timeSyncService->Now();
            if (participantIsSync[participantId] && now >= 0ns)
            {
                auto delta = now - dataMessageEvent.timestamp;
                if (delta > simStepSize)
                {
                    testParticipant.lifecycleService->Stop("Fail with chronology error");

                    FAIL() << "Chronology error: Participant " << testParticipant.name << " received message from "
                           << participantNames[participantId] << " with timestamp "
                           << dataMessageEvent.timestamp.count() << " while this participant's time is " << now.count();
                }
            }

            testParticipant.receivedIds.insert(dataMessageEvent.data[0]);
            // No self delivery: Expect expectedReceptions-1 receptions
            if (testParticipant.receivedIds.size() == expectedReceptions - 1)
            {
                testParticipant.i.allReceived = true;
                testParticipant.allReceivedPromise.set_value();
            }
        });

        timeSyncService->SetSimulationStepHandler(
            [&testParticipant, this](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
            testParticipant.now = now;
            testParticipant.publisher->Publish(std::vector<uint8_t>{testParticipant.id});
            if (!testParticipant.simtimePassed && now > _simtimeToPass)
            {
                testParticipant.simtimePassed = true;
                testParticipant.simtimePassedPromise.set_value();
            }
        }, simStepSize);
        auto finalStateFuture = testParticipant.lifecycleService->StartLifecycle();
        finalStateFuture.get();

        testParticipant.participant.reset();
    }

    void AsyncParticipantThread(TestParticipant& testParticipant)
    {
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> config{nullptr};
        if (logging)
        {
            config = SilKit::Config::MakeParticipantConfigurationWithLogging(logLevel);
        }
        else
        {
            config = SilKit::Config::MakeEmptyParticipantConfiguration();
        }

        testParticipant.participant = SilKit::CreateParticipant(config, testParticipant.name, _registryUri);
        if (testParticipant.lifeCycleOperationMode != OperationMode::Invalid)
        {
            testParticipant.lifecycleService =
                testParticipant.participant->CreateLifecycleService({testParticipant.lifeCycleOperationMode});
        }
        auto systemMonitor = testParticipant.participant->CreateSystemMonitor();
        systemMonitor->AddParticipantStatusHandler([&testParticipant, this](ParticipantStatus status) {
            testParticipant.SetMonitoredParticipantState(status.participantName, status.state);
            if (status.participantName != testParticipant.name)
            {
                // we're only tracking the status of synchronized participants at the moment.
                this->callbacks.ParticipantStateHandler(status.state);
            }
            if (status.participantName == testParticipant.name && status.state == ParticipantState::Running
                && !testParticipant.i.runningStateReached)
            {
                testParticipant.i.runningStateReached = true;
                testParticipant.runningStatePromise.set_value();
            }
        });

        SilKit::Services::PubSub::PubSubSpec dataSpec{topic, mediaType};
        SilKit::Services::PubSub::PubSubSpec matchingDataSpec{topic, mediaType};
        testParticipant.publisher = testParticipant.participant->CreateDataPublisher("TestPublisher", dataSpec, 0);
        testParticipant.subscriber = testParticipant.participant->CreateDataSubscriber(
            "TestSubscriber", matchingDataSpec,
            [&testParticipant](IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent) {
            if (!testParticipant.i.allReceived)
            {
                auto participantId = dataMessageEvent.data[0];
                if (participantId != testParticipant.id)
                {
                    testParticipant.receivedIds.insert(dataMessageEvent.data[0]);
                    // No self delivery: Expect expectedReceptions-1 receptions
                    if (testParticipant.receivedIds.size() == expectedReceptions - 1)
                    {
                        testParticipant.i.allReceived = true;
                        testParticipant.allReceivedPromise.set_value();
                    }
                }
            }
        });

        auto runTask = [&testParticipant]() {
            while (testParticipant.i._runAsync)
            {
                testParticipant.publisher->Publish(std::vector<uint8_t>{testParticipant.id});
                std::this_thread::sleep_for(asyncDelayBetweenPublication);
                if (testParticipant.lifeCycleOperationMode != OperationMode::Invalid && testParticipant.i.stopRequested)
                {
                    testParticipant.lifecycleService->Stop("Stop requested");
                    // TODO: Causes TSAN issue that is not fully understood, will be fixed in SILKIT-1361.
                    //testParticipant.i.stopRequested = false;
                    break;
                }
            }
        };

        if (testParticipant.lifeCycleOperationMode != OperationMode::Invalid)
        {
            ParticipantThread runTaskThread(runTask);

            auto finalStateFuture = testParticipant.lifecycleService->StartLifecycle();
            finalStateFuture.get();

            if (!runTaskThread.WaitForThreadExit(communicationTimeout))
            {
                FAIL() << "Test Failure: runTaskThread timed out";
            }

            testParticipant.i.stopRequested = false;
        }
        else
        {
            runTask();
        }

        testParticipant.participant.reset();
    }

    void CreateSystemControllerParticipant(const std::vector<std::string>& required)
    {
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> config{nullptr};
        if (logging)
        {
            config = SilKit::Config::MakeParticipantConfigurationWithLogging(logLevel);
        }
        else
        {
            config = SilKit::Config::MakeEmptyParticipantConfiguration();
        }

        systemControllerParticipant.participant =
            SilKit::CreateParticipant(config, systemControllerParticipantName, _registryUri);

        systemControllerParticipant.systemController =
            SilKit::Experimental::Participant::CreateSystemController(systemControllerParticipant.participant.get());

        systemControllerParticipant.systemMonitor = systemControllerParticipant.participant->CreateSystemMonitor();
        systemControllerParticipant.systemMonitor->AddParticipantStatusHandler(
            [this](const Services::Orchestration::ParticipantStatus& status) {
            {
                if (status.participantName == systemControllerParticipantName
                    && status.state == ParticipantState::Running && !systemControllerParticipant.i.runningStateReached)
                {
                    systemControllerParticipant.i.runningStateReached = true;
                    systemControllerParticipant.runningStatePromise.set_value();
                }
                else if (status.participantName == systemControllerParticipantName
                         && status.state == ParticipantState::Paused
                         && !systemControllerParticipant.i.pausedStateReached)
                {
                    systemControllerParticipant.i.pausedStateReached = true;
                    systemControllerParticipant.pausedStatePromise.set_value();
                }
                else if (status.participantName == systemControllerParticipantName
                         && status.state == ParticipantState::CommunicationInitialized
                         && !systemControllerParticipant.i.communicationInitializedStateReached)
                {
                    systemControllerParticipant.i.communicationInitializedStateReached = true;
                    systemControllerParticipant.communicationInitializedStatePromise.set_value();
                }
            }
        });
        systemControllerParticipant.lifecycleService = systemControllerParticipant.participant->CreateLifecycleService(
            {SilKit::Services::Orchestration::OperationMode::Coordinated});

        systemControllerParticipant.lifecycleService->SetAbortHandler(
            SilKit::Util::bind_method(&callbacks, &Callbacks::AbortHandler));
        systemControllerParticipant.systemController->SetWorkflowConfiguration({required});

        systemControllerParticipant.systemMonitor->AddSystemStateHandler(
            [this](SystemState newState) { SystemStateHandler(newState); });

        if (verbose)
        {
            std::cout << ">> CreateSystemControllerParticipant done" << std::endl;
        }
    }

    void StartSystemController()
    {
        systemControllerParticipant.finalState = systemControllerParticipant.lifecycleService->StartLifecycle();
        if (verbose)
        {
            std::cout << ">> SystemController started" << std::endl;
        }
        systemControllerParticipant.finalState.get();
        if (verbose)
        {
            std::cout << ">> SystemController done" << std::endl;
        }
    }

    void RunSystemController(const std::vector<std::string>& requiredParticipants,
                             std::function<void()> setup = nullptr)
    {
        if (!requiredParticipants.empty())
        {
            std::vector<std::string> required{};
            for (auto&& p : requiredParticipants)
            {
                required.push_back(p);
            }
            required.push_back(systemControllerParticipantName);

            participantThread_SystemController = ParticipantThread{[this, required, setup] {
                CreateSystemControllerParticipant(required);
                if (setup)
                {
                    setup();
                    if (verbose)
                    {
                        std::cout << ">> SystemController setup done" << std::endl;
                    }
                }
                StartSystemController();
            }};
        }
    }

    void StopSystemController()
    {
        systemControllerParticipant.lifecycleService->Stop("Stop Test.");
        participantThread_SystemController.WaitForThreadExit();
    }

    void PauseSystemController()
    {
        systemControllerParticipant.lifecycleService->Pause("Pause Test.");
        systemControllerParticipant.AwaitPaused();
    }

    void AbortSystemController()
    {
        systemControllerParticipant.systemController->AbortSimulation();
        participantThread_SystemController.WaitForThreadExit();
    }

    void RunRegistry()
    {
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> config{nullptr};
        config = SilKit::Config::MakeEmptyParticipantConfiguration();
        _registry = SilKit::Vendor::Vector::CreateSilKitRegistry(config);
        _registryUri = _registry->StartListening("silkit://127.0.0.1:0");
    }

    void StopRegistry()
    {
        _registry.reset();
    }

    void RunParticipants(std::list<TestParticipant>& participants, std::string set = "A")
    {
        for (auto& p : participants)
        {
            if (p.timeMode == TimeMode::Async)
            {
                p.i._runAsync = true;

                if (p.lifeCycleOperationMode == OperationMode::Invalid)
                {
                    _participantThreads_Async_Invalid.emplace_back([this, &p] { AsyncParticipantThread(p); });
                }
                else if (p.lifeCycleOperationMode == OperationMode::Autonomous)
                {
                    _participantThreads_Async_Autonomous.emplace_back([this, &p] { AsyncParticipantThread(p); });
                }
                else if (p.lifeCycleOperationMode == OperationMode::Coordinated)
                {
                    _participantThreads_Async_Coordinated.emplace_back([this, &p] { AsyncParticipantThread(p); });
                }
            }
            else if (p.timeMode == TimeMode::Sync)
            {
                if (p.lifeCycleOperationMode == OperationMode::Invalid)
                {
                    _participantThreads_Sync_Invalid.emplace_back([this, &p] { SyncParticipantThread(p); });
                }
                else if (p.lifeCycleOperationMode == OperationMode::Autonomous)
                {
                    if (set == "A")
                    {
                        _participantThreads_Sync_AutonomousA.emplace_back([this, &p] { SyncParticipantThread(p); });
                    }
                    else if (set == "B")
                    {
                        _participantThreads_Sync_AutonomousB.emplace_back([this, &p] { SyncParticipantThread(p); });
                    }
                }
                else if (p.lifeCycleOperationMode == OperationMode::Coordinated)
                {
                    _participantThreads_Sync_Coordinated.emplace_back([this, &p] { SyncParticipantThread(p); });
                }
            }
        }
    }

    class ParticipantThread
    {
        struct ThreadMain
        {
            void operator()(std::function<void()> f)
            {
                try
                {
                    f();
                }
                catch (...)
                {
                    promise.set_exception(std::current_exception());
                    return;
                }

                promise.set_value();
            }

            std::promise<void> promise;
        };

    public:
        ParticipantThread() {}
        ParticipantThread(ParticipantThread&&) = default;
        ParticipantThread(const ParticipantThread&) = delete;

        ParticipantThread& operator=(ParticipantThread&&) = default;
        ParticipantThread& operator=(const ParticipantThread&) = delete;

        template <typename T>
        ParticipantThread(T&& t)
        {
            ThreadMain threadMain;
            shutdownFuture = threadMain.promise.get_future();
            thread = std::thread{std::move(threadMain), std::forward<T>(t)};
        }

        ~ParticipantThread()
        {
            if (shutdownFuture.valid())
            {
                try
                {
                    shutdownFuture.wait();
                }
                catch (...)
                {
                }
            }
            if (thread.joinable())
            {
                try
                {
                    thread.join();
                }
                catch (...)
                {
                }
            }
        }

        bool WaitForThreadExit(std::chrono::milliseconds timeout)
        {
            if (shutdownFuture.valid())
            {
                auto futureStatus = shutdownFuture.wait_for(timeout);
                if (futureStatus != std::future_status::ready)
                {
                    return false;
                }
                else
                {
                    shutdownFuture.get();
                    if (thread.joinable())
                    {
                        thread.join();
                    }
                    return true;
                }
            }
            return false;
        }

        bool WaitForThreadExit()
        {
            if (shutdownFuture.valid())
            {
                shutdownFuture.get();
                if (thread.joinable())
                {
                    thread.join();
                }
                return true;
            }
            return false;
        }

    private:
        std::thread thread;
        std::future<void> shutdownFuture;
    };

    void JoinParticipantThreads(std::vector<ParticipantThread>& threads)
    {
        for (auto&& thread : threads)
        {
            thread.WaitForThreadExit();
        }
        threads.clear();
    }

    void SetUp() override
    {
        globalParticipantIndex = 0;
        participantNames.clear();
        participantIsSync.clear();
    }

protected:
    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> _registry;

    SystemControllerParticipant systemControllerParticipant;
    ParticipantThread participantThread_SystemController;

    std::vector<ParticipantThread> _participantThreads_Sync_Invalid;
    std::vector<ParticipantThread> _participantThreads_Sync_AutonomousA;
    std::vector<ParticipantThread> _participantThreads_Sync_AutonomousB;
    std::vector<ParticipantThread> _participantThreads_Sync_Coordinated;

    std::vector<ParticipantThread> _participantThreads_Async_Invalid;
    std::vector<ParticipantThread> _participantThreads_Async_Autonomous;
    std::vector<ParticipantThread> _participantThreads_Async_Coordinated;

    std::chrono::seconds _simtimeToPass{3s};
    bool _runAsync{true};

    std::string _registryUri{"undefined registry uri"};

    Callbacks callbacks;
};
