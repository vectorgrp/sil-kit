/* Copyright (c) 2022 Vector Informatik GmbH

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
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <list>

#include "GetTestPid.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"

#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/pubsub/PubSubSpec.hpp"

#include "ConfigurationTestUtils.hpp"
#include "IParticipantInternal.hpp"
#include "CreateParticipantImpl.hpp"

namespace {

using namespace std::chrono_literals;
using namespace SilKit;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Config;
using namespace SilKit::Services::PubSub;

const std::string systemControllerParticipantName{"systemControllerParticipant"};
const std::string topic{"Topic"};
const std::string mediaType{"A"};
static size_t expectedReceptions;
std::chrono::milliseconds communicationTimeout{8000ms};
std::chrono::milliseconds asyncDelayBetweenPublication{50ms};
std::chrono::seconds simStepSize{1s};

static size_t globalParticipantIndex = 0;
static std::vector<std::string> participantNames{};
static std::map<size_t, bool> participantIsSync{};

const bool verbose = true;
const bool logging = false;
const Services::Logging::Level logLevel = Services::Logging::Level::Info;

enum class TimeMode
{
    Async,
    Sync
};

class ITest_HopOnHopOff : public testing::Test
{
protected:
    ITest_HopOnHopOff() {}

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
                , runAsync{other.runAsync.load()}

                , stopRequested{other.stopRequested.load()}
            {
            }

            ImmovableMembers& operator=(ImmovableMembers&& other) noexcept
            {
                if (this != &other)
                {
                    allReceived = other.allReceived.load();
                    runAsync = other.runAsync.load();
                    stopRequested = other.stopRequested.load();
                }

                return *this;
            }

            std::atomic<bool> allReceived{false};
            std::atomic<bool> runAsync{true};
            std::atomic<bool> stopRequested{false};
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
    };

    struct SystemControllerParticipant
    {
        struct ImmovableMembers
        {
            ImmovableMembers() = default;
            ImmovableMembers(ImmovableMembers&& other) noexcept
                : runningStateReached{other.runningStateReached.load()}
            {
            }

            ImmovableMembers& operator=(ImmovableMembers&& other) noexcept
            {
                if (this != &other)
                {
                    runningStateReached = other.runningStateReached.load();
                }

                return *this;
            }

            std::atomic<bool> runningStateReached{false};
        };

        ImmovableMembers i{};
        std::promise<void> runningStatePromise;
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

        case SystemState::Running: break;

        default: break;
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

        testParticipant.participant = SilKit::CreateParticipant(config, testParticipant.name, registryUri);
        testParticipant.lifecycleService =
            testParticipant.participant->CreateLifecycleService({testParticipant.lifeCycleOperationMode});
        auto* timeSyncService = testParticipant.lifecycleService->CreateTimeSyncService();
        auto* logger = testParticipant.participant->GetLogger();

        SilKit::Services::PubSub::PubSubSpec dataSpec{topic, mediaType};
        SilKit::Services::PubSub::PubSubSpec matchingDataSpec{topic, mediaType};
        testParticipant.publisher = testParticipant.participant->CreateDataPublisher("TestPublisher", dataSpec, 0);
        testParticipant.subscriber = testParticipant.participant->CreateDataSubscriber(
            "TestSubscriber", matchingDataSpec,
            [logger, timeSyncService, &testParticipant](IDataSubscriber* /*subscriber*/,
                                                const DataMessageEvent& dataMessageEvent) {
                auto participantId = dataMessageEvent.data[0];

                if (testParticipant.i.allReceived || participantId == testParticipant.id)
                    return;

                auto now = timeSyncService->Now();

                std::stringstream ss;
                ss << testParticipant.name << ": receive from " << participantNames[participantId]
                   << " at now=" << now.count() << " with timestamp=" << dataMessageEvent.timestamp.count();
                logger->Info(ss.str());

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
            [logger, &testParticipant, this](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                testParticipant.now = now;
                std::stringstream ss;
                ss << "now=" << now.count() / 1e9 << "s";
                logger->Info(ss.str());
                testParticipant.publisher->Publish(std::vector<uint8_t>{testParticipant.id});
                if (!testParticipant.simtimePassed && now > simtimeToPass)
                {
                    testParticipant.simtimePassed = true;
                    testParticipant.simtimePassedPromise.set_value();
                }
            },
            simStepSize);
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

        testParticipant.participant = SilKit::CreateParticipant(config, testParticipant.name, registryUri);
        if (testParticipant.lifeCycleOperationMode != OperationMode::Invalid)
        {
            testParticipant.lifecycleService =
                testParticipant.participant->CreateLifecycleService({testParticipant.lifeCycleOperationMode});
        }
        auto systemMonitor = testParticipant.participant->CreateSystemMonitor();
        systemMonitor->AddParticipantStatusHandler([&testParticipant](ParticipantStatus status) {
            testParticipant.participantStates[status.participantName] = status.state;
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
            while (testParticipant.i.runAsync)
            {
                testParticipant.publisher->Publish(std::vector<uint8_t>{testParticipant.id});
                std::this_thread::sleep_for(asyncDelayBetweenPublication);
                if (testParticipant.i.stopRequested)
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
            std::thread runTaskThread{runTask};
            runTaskThread.detach();

            auto finalStateFuture = testParticipant.lifecycleService->StartLifecycle();
            finalStateFuture.get();
            if (runTaskThread.joinable())
            {
                runTaskThread.join();
            }
        }
        else
        {
            runTask();
        }

        testParticipant.participant.reset();
    }

    void SystemControllerParticipantThread(const std::vector<std::string>& required)
    {
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> config{nullptr};
        if (logging)
        {
            config = SilKit::Config::MakeParticipantConfigurationWithLogging(logLevel);
        }
        else
        {
            config = SilKit::Config::MakeEmptyParticipantConfigurationImpl();
        }

        systemControllerParticipant.participant =
            SilKit::CreateParticipantImpl(config, systemControllerParticipantName, registryUri);

        auto participantInternal =
            dynamic_cast<SilKit::Core::IParticipantInternal*>(systemControllerParticipant.participant.get());
        systemControllerParticipant.systemController = participantInternal->GetSystemController();

        systemControllerParticipant.systemMonitor = systemControllerParticipant.participant->CreateSystemMonitor();
        systemControllerParticipant.systemMonitor->AddParticipantStatusHandler(
            [this](const Services::Orchestration::ParticipantStatus& status) {
                {
                    if (status.participantName == systemControllerParticipantName
                            && status.state == ParticipantState::Running
                            && !systemControllerParticipant.i.runningStateReached)
                    {
                        systemControllerParticipant.i.runningStateReached = true;
                        systemControllerParticipant.runningStatePromise.set_value();
                    }
                }
            });
        systemControllerParticipant.lifecycleService = systemControllerParticipant.participant->CreateLifecycleService(
            {SilKit::Services::Orchestration::OperationMode::Coordinated});

        systemControllerParticipant.systemController->SetWorkflowConfiguration({required});

        systemControllerParticipant.systemMonitor->AddSystemStateHandler([this](SystemState newState) {
            SystemStateHandler(newState);
        });

        systemControllerParticipant.finalState = systemControllerParticipant.lifecycleService->StartLifecycle();
        systemControllerParticipant.finalState.get();
    }

    void RunSystemController(const std::vector<std::string>& requiredParticipants)
    {
        if (!requiredParticipants.empty())
        {
            std::vector<std::string> required{};
            for (auto&& p : requiredParticipants)
            {
                required.push_back(p);
            }
            required.push_back(systemControllerParticipantName);

            participantThread_SystemController = ParticipantThread{[this, required] {
                SystemControllerParticipantThread(required);
            }};
        }
    }

    void StopSystemController()
    {
        systemControllerParticipant.lifecycleService->Stop("Stop Test.");
        participantThread_SystemController.shutdownFuture.get();
        participantThread_SystemController.thread.join();
    }

    void RunRegistry()
    {
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> config{nullptr};
        config = SilKit::Config::MakeEmptyParticipantConfiguration();
        registry = SilKit::Vendor::Vector::CreateSilKitRegistry(config);
        registry->StartListening(registryUri);
    }

    void StopRegistry() { registry.reset(); }

    void RunParticipants(std::list<TestParticipant>& participants, std::string set = "A")
    {
        for (auto& p : participants)
        {
            if (p.timeMode == TimeMode::Async)
            {
                p.i.runAsync = true;

                if (p.lifeCycleOperationMode == OperationMode::Invalid)
                {
                    participantThreads_Async_Invalid.emplace_back([this, &p] {
                        AsyncParticipantThread(p);
                    });
                }
                else if (p.lifeCycleOperationMode == OperationMode::Autonomous)
                {
                    participantThreads_Async_Autonomous.emplace_back([this, &p] {
                        AsyncParticipantThread(p);
                    });
                }
                else if (p.lifeCycleOperationMode == OperationMode::Coordinated)
                {
                    participantThreads_Async_Coordinated.emplace_back([this, &p] {
                        AsyncParticipantThread(p);
                    });
                }
            }
            else if (p.timeMode == TimeMode::Sync)
            {
                if (p.lifeCycleOperationMode == OperationMode::Invalid)
                {
                    participantThreads_Sync_Invalid.emplace_back([this, &p] {
                        SyncParticipantThread(p);
                    });
                }
                else if (p.lifeCycleOperationMode == OperationMode::Autonomous)
                {
                    if (set == "A")
                    {
                        participantThreads_Sync_AutonomousA.emplace_back([this, &p] {
                            SyncParticipantThread(p);
                        });
                    }
                    else if (set == "B")
                    {
                        participantThreads_Sync_AutonomousB.emplace_back([this, &p] {
                            SyncParticipantThread(p);
                        });
                    }
                }
                else if (p.lifeCycleOperationMode == OperationMode::Coordinated)
                {
                    participantThreads_Sync_Coordinated.emplace_back([this, &p] {
                        SyncParticipantThread(p);
                    });
                }
            }
        }
    }

    struct ParticipantThread
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

        std::thread thread;
        std::future<void> shutdownFuture;
    };

    void JoinParticipantThreads(std::vector<ParticipantThread>& threads)
    {
        for (auto&& thread : threads)
        {
            if (thread.shutdownFuture.valid())
            {
                thread.shutdownFuture.get();
            }
        }
        threads.clear();
    }

    void SetUp() override { 
        globalParticipantIndex = 0;
        participantNames.clear();
        participantIsSync.clear();
        registryUri = MakeTestRegistryUri(); 
    }

protected:
    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> registry;

    SystemControllerParticipant systemControllerParticipant;
    ParticipantThread participantThread_SystemController;

    std::vector<ParticipantThread> participantThreads_Sync_Invalid;
    std::vector<ParticipantThread> participantThreads_Sync_AutonomousA;
    std::vector<ParticipantThread> participantThreads_Sync_AutonomousB;
    std::vector<ParticipantThread> participantThreads_Sync_Coordinated;

    std::vector<ParticipantThread> participantThreads_Async_Invalid;
    std::vector<ParticipantThread> participantThreads_Async_Autonomous;
    std::vector<ParticipantThread> participantThreads_Async_Coordinated;

    std::chrono::seconds simtimeToPass{3s};
    bool runAsync{true};

    std::string registryUri;
};


TEST_F(ITest_HopOnHopOff, test_GracefulShutdown)
{
    RunRegistry();

    std::list<TestParticipant> syncParticipants;
    syncParticipants.push_back({"SyncParticipant1", TimeMode::Sync, OperationMode::Coordinated});
    syncParticipants.push_back({"SyncParticipant2", TimeMode::Sync, OperationMode::Coordinated});

    std::list<TestParticipant> monitorParticipants;
    monitorParticipants.push_back({"MonitorParticipant1", TimeMode::Async, OperationMode::Autonomous});

    expectedReceptions = syncParticipants.size() + monitorParticipants.size();

    std::vector<std::string> required{};
    for (auto&& p : syncParticipants)
    {
        required.push_back(p.name);
    }
    RunSystemController(required);

    RunParticipants(monitorParticipants);
    RunParticipants(syncParticipants);

    // Each participant knows that all other participants have started.
    for (auto& p : syncParticipants)
    {
        p.AwaitCommunication();
    }
    for (auto& p : monitorParticipants)
    {
        p.AwaitCommunication();
    }
    systemControllerParticipant.AwaitRunning();

    // Stop via system controller participant
    StopSystemController();

    // Wait for coordinated to end
    JoinParticipantThreads(participantThreads_Sync_Coordinated);

    // Expect shutdown state
    EXPECT_EQ(monitorParticipants.front().participantStates["SyncParticipant1"], ParticipantState::Shutdown);
    EXPECT_EQ(monitorParticipants.front().participantStates["SyncParticipant2"], ParticipantState::Shutdown);

    monitorParticipants.front().i.stopRequested = true;
    JoinParticipantThreads(participantThreads_Async_Autonomous);

    StopRegistry();
}


TEST_F(ITest_HopOnHopOff, test_Async_HopOnHopOff_ToSynced)
{
    RunRegistry();

    std::list<TestParticipant> syncParticipants;
    syncParticipants.push_back({"SyncParticipant1", TimeMode::Sync, OperationMode::Coordinated});
    syncParticipants.push_back({"SyncParticipant2", TimeMode::Sync, OperationMode::Coordinated});

    std::list<TestParticipant> asyncParticipants;
    asyncParticipants.push_back({"AsyncParticipant1", TimeMode::Async, OperationMode::Invalid});
    asyncParticipants.push_back({"AsyncParticipant2", TimeMode::Async, OperationMode::Invalid});
    expectedReceptions = syncParticipants.size() + asyncParticipants.size();

    std::vector<std::string> required{};
    for (auto&& p : syncParticipants)
    {
        required.push_back(p.name);
    }
    RunSystemController(required);

    RunParticipants(syncParticipants);

    if (verbose)
    {
        std::cout << "Await simtime progress" << std::endl;
    }
    // Await simtime progress
    for (auto& p : syncParticipants)
    {
        auto futureStatus = p.simtimePassedPromise.get_future().wait_for(communicationTimeout);
        EXPECT_EQ(futureStatus, std::future_status::ready)
            << "Test Failure: Sync participants didn't achieve sim time barrier";
    }

    for (int i = 0; i < 3; i++)
    {
        if (verbose)
        {
            std::cout << ">> Cycle " << i + 1 << "/3" << std::endl;
            std::cout << ">> Hop on async participants" << std::endl;
        }

        // Hop on with async participant
        RunParticipants(asyncParticipants);

        if (verbose)
        {
            std::cout << ">> Await successful communication of async/sync participants" << std::endl;
        }
        // Await successful communication of async/sync participants
        for (auto& p : syncParticipants)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants)
            p.AwaitCommunication();

        if (verbose)
        {
            std::cout << ">> Hop off async participants" << std::endl;
        }
        // Hop off: Stop while-loop of async participants
        for (auto& p : asyncParticipants)
            p.i.runAsync = false;
        JoinParticipantThreads(participantThreads_Async_Invalid);

        // Reset communication and wait for reception once more for remaining sync participants
        expectedReceptions = syncParticipants.size();
        for (auto& p : syncParticipants)
            p.ResetReception();

        if (verbose)
        {
            std::cout << ">> Await successful communication of remaining sync participants" << std::endl;
        }
        for (auto& p : syncParticipants)
            p.AwaitCommunication();

        // Reset communication to repeat the cycle
        expectedReceptions = syncParticipants.size() + asyncParticipants.size();
        for (auto& p : syncParticipants)
            p.ResetReception();
        for (auto& p : asyncParticipants)
            p.ResetReception();
    }
    if (verbose)
    {
        std::cout << ">> Cycles done" << std::endl;
    }

    // Shutdown coordinated participants
    for (auto& p : syncParticipants)
        p.lifecycleService->Stop("Hop Off");
    JoinParticipantThreads(participantThreads_Sync_Coordinated);

    StopSystemController();
    StopRegistry();
}

TEST_F(ITest_HopOnHopOff, test_Async_reconnect_first_joined)
{
    std::list<TestParticipant> asyncParticipants1;
    asyncParticipants1.push_back({"AsyncParticipant1", TimeMode::Async, OperationMode::Invalid});
    std::list<TestParticipant> asyncParticipants2;
    asyncParticipants2.push_back({"AsyncParticipant2", TimeMode::Async, OperationMode::Invalid});
    expectedReceptions = asyncParticipants1.size() + asyncParticipants2.size();

    // No lifecycle, only registry needed (no systemController)
    RunRegistry();

    for (int i = 0; i < 3; i++)
    {
        // Start with asyncParticipant1
        RunParticipants(asyncParticipants1);
        // Async2 is second
        RunParticipants(asyncParticipants2);

        // Await successful communication of async participants
        for (auto& p : asyncParticipants1)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants2)
            p.AwaitCommunication();

        // Reset communication
        expectedReceptions = 2;
        for (auto& p : asyncParticipants1)
            p.ResetReception();
        for (auto& p : asyncParticipants2)
            p.ResetReception();

        // Hop off with asyncParticipants1
        asyncParticipants1.front().i.runAsync = false;
        participantThreads_Async_Invalid[0].shutdownFuture.get();

        // Reconnect with asyncParticipant1
        RunParticipants(asyncParticipants1);

        // Await successful communication of async participants
        for (auto& p : asyncParticipants1)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants2)
            p.AwaitCommunication();

        // Reset communication
        expectedReceptions = 2;
        for (auto& p : asyncParticipants1)
            p.ResetReception();
        for (auto& p : asyncParticipants2)
            p.ResetReception();

        // Disconnect with both
        for (auto& p : asyncParticipants1)
            p.i.runAsync = false;
        for (auto& p : asyncParticipants2)
            p.i.runAsync = false;

        JoinParticipantThreads(participantThreads_Async_Invalid);
    }

    StopRegistry();
}

TEST_F(ITest_HopOnHopOff, test_Async_reconnect_second_joined)
{
    std::list<TestParticipant> asyncParticipants1;
    asyncParticipants1.push_back({"AsyncParticipant1", TimeMode::Async, OperationMode::Invalid});
    std::list<TestParticipant> asyncParticipants2;
    asyncParticipants2.push_back({"AsyncParticipant2", TimeMode::Async, OperationMode::Invalid});
    expectedReceptions = asyncParticipants1.size() + asyncParticipants2.size();

    // No lifecycle, only registry needed (no systemController)
    RunRegistry();

    for (int i = 0; i < 3; i++)
    {
        // Start with asyncParticipant1
        RunParticipants(asyncParticipants1);
        // Async2 is second
        RunParticipants(asyncParticipants2);

        // Await successful communication of async participants
        for (auto& p : asyncParticipants1)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants2)
            p.AwaitCommunication();

        // Reset communication
        expectedReceptions = 2;
        for (auto& p : asyncParticipants1)
            p.ResetReception();
        for (auto& p : asyncParticipants2)
            p.ResetReception();

        // Hop off with asyncParticipants2
        asyncParticipants2.front().i.runAsync = false;
        participantThreads_Async_Invalid[1].shutdownFuture.get();
        //participantThreads_Async_Invalid.erase(participantThreads_Async_Invalid.begin()+1);

        // Reconnect with asyncParticipant2
        RunParticipants(asyncParticipants2);

        // Await successful communication of async participants
        for (auto& p : asyncParticipants1)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants2)
            p.AwaitCommunication();

        // Reset communication
        expectedReceptions = 2;
        for (auto& p : asyncParticipants1)
            p.ResetReception();
        for (auto& p : asyncParticipants2)
            p.ResetReception();

        // Disconnect with both
        for (auto& p : asyncParticipants1)
            p.i.runAsync = false;
        for (auto& p : asyncParticipants2)
            p.i.runAsync = false;

        JoinParticipantThreads(participantThreads_Async_Invalid);
    }

    StopRegistry();
}

TEST_F(ITest_HopOnHopOff, test_Async_HopOnHopOff_ToEmpty)
{
    RunRegistry();

    std::list<TestParticipant> asyncParticipants;
    asyncParticipants.push_back({"AsyncParticipant1", TimeMode::Async, OperationMode::Invalid});
    asyncParticipants.push_back({"AsyncParticipant2", TimeMode::Async, OperationMode::Invalid});
    expectedReceptions = asyncParticipants.size();

    for (int i = 0; i < 3; i++)
    {
        // Hop on with async participant on empty sim
        RunParticipants(asyncParticipants);

        // Await successful communication of async/sync participants
        for (auto& p : asyncParticipants)
            p.AwaitCommunication();

        // Hop off async participants
        for (auto& p : asyncParticipants)
            p.i.runAsync = false;

        JoinParticipantThreads(participantThreads_Async_Invalid);

        // Reset communication to repeat the cycle
        for (auto& p : asyncParticipants)
            p.ResetReception();
    }

    StopRegistry();
}

TEST_F(ITest_HopOnHopOff, test_HopOnHopOff_Autonomous_To_Coordinated)
{
    RunRegistry();

    // The coordinated and required participants
    std::list<TestParticipant> syncParticipantsCoordinated;
    syncParticipantsCoordinated.push_back({"SyncCoordinated1", TimeMode::Sync, OperationMode::Coordinated});
    syncParticipantsCoordinated.push_back({"SyncCoordinated2", TimeMode::Sync, OperationMode::Coordinated});

    // The autonomous, non-required participants that will hop on/off
    std::list<TestParticipant> syncParticipantsAutonomous;
    syncParticipantsAutonomous.push_back({"SyncAutonomous1", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomous.push_back({"SyncAutonomous2", TimeMode::Sync, OperationMode::Autonomous});

    // Additional async, non-required participants for testing the mixture
    std::list<TestParticipant> asyncParticipants;
    asyncParticipants.push_back({"ASync1", TimeMode::Async, OperationMode::Invalid});

    // Coordinated are required
    std::vector<std::string> required{};
    for (auto&& p : syncParticipantsCoordinated)
    {
        required.push_back(p.name);
    }
    RunSystemController(required);

    // Run coordinated
    expectedReceptions = syncParticipantsCoordinated.size() + asyncParticipants.size();
    RunParticipants(asyncParticipants);
    RunParticipants(syncParticipantsCoordinated);

    for (int i = 0; i < 3; i++)
    {
        if (verbose)
        {
            std::cout << ">> Cycle " << i + 1 << "/3" << std::endl;
        }

        if (verbose)
        {
            std::cout << ">> Await successful communication of coordinated participants" << std::endl;
        }

        // Await successful communication of avaliable participants (coordinated, async)
        for (auto& p : syncParticipantsCoordinated)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants)
            p.AwaitCommunication();

        // Reset communication
        expectedReceptions =
            syncParticipantsCoordinated.size() + syncParticipantsAutonomous.size() + asyncParticipants.size();
        for (auto& p : syncParticipantsCoordinated)
            p.ResetReception();
        for (auto& p : asyncParticipants)
            p.ResetReception();

        if (verbose)
        {
            std::cout << ">> Hop on with autonomous participants" << std::endl;
        }

        // Hop on with autonomous
        RunParticipants(syncParticipantsAutonomous);

        if (verbose)
        {
            std::cout << ">> Await successful communication of all participants" << std::endl;
        }
        for (auto& p : syncParticipantsCoordinated)
            p.AwaitCommunication();
        for (auto& p : syncParticipantsAutonomous)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants)
            p.AwaitCommunication();

        if (verbose)
        {
            std::cout << ">> Hop off with autonomous participants" << std::endl;
        }

        for (auto& p : syncParticipantsAutonomous)
            p.lifecycleService->Stop("Hop Off");

        JoinParticipantThreads(participantThreads_Sync_AutonomousA);

        // Reset communication to repeat the cycle
        expectedReceptions = syncParticipantsCoordinated.size() + asyncParticipants.size();
        for (auto& p : syncParticipantsCoordinated)
            p.ResetReception();
        for (auto& p : syncParticipantsAutonomous)
            p.ResetReception();
        for (auto& p : asyncParticipants)
            p.ResetReception();
    }

    // Shutdown async participants
    for (auto& p : asyncParticipants)
        p.i.runAsync = false;
    JoinParticipantThreads(participantThreads_Async_Invalid);

    // Shutdown coordinated participants
    for (auto& p : syncParticipantsCoordinated)
        p.lifecycleService->Stop("Hop Off");
    JoinParticipantThreads(participantThreads_Sync_Coordinated);

    StopSystemController();
    StopRegistry();
}

TEST_F(ITest_HopOnHopOff, test_HopOnHopOff_Autonomous_To_Autonomous)
{
    RunRegistry();

    // The autonomous, non-required participant that is there from the start
    std::list<TestParticipant> syncParticipantsAutonomousA;
    syncParticipantsAutonomousA.push_back({"SyncAutonomousA1", TimeMode::Sync, OperationMode::Autonomous});

    // The autonomous, non-required participants that will hop on/off
    std::list<TestParticipant> syncParticipantsAutonomousB;
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB1", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB2", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB3", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB4", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB5", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB6", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB7", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB8", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB9", TimeMode::Sync, OperationMode::Autonomous});


    // Additional async, non-required participants for testing the mixture
    std::list<TestParticipant> asyncParticipants;
    asyncParticipants.push_back({"ASync1", TimeMode::Async, OperationMode::Invalid});

    // Run coordinated
    expectedReceptions = syncParticipantsAutonomousA.size() + asyncParticipants.size();
    RunParticipants(asyncParticipants);
    RunParticipants(syncParticipantsAutonomousA);

    for (int i = 0; i < 3; i++)
    {
        if (verbose)
        {
            std::cout << ">> Cycle " << i + 1 << "/3" << std::endl;
        }

        if (verbose)
        {
            std::cout << ">> Await successful communication of coordinated participants" << std::endl;
        }

        // Await successful communication of avaliable participants (autonomous A, async)
        for (auto& p : syncParticipantsAutonomousA)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants)
            p.AwaitCommunication();

        // Reset communication
        expectedReceptions =
            syncParticipantsAutonomousA.size() + syncParticipantsAutonomousB.size() + asyncParticipants.size();
        for (auto& p : syncParticipantsAutonomousA)
            p.ResetReception();
        for (auto& p : asyncParticipants)
            p.ResetReception();

        if (verbose)
        {
            std::cout << ">> Hop on with autonomous participants" << std::endl;
        }

        // Hop on with autonomous B
        RunParticipants(syncParticipantsAutonomousB, "B");

        if (verbose)
        {
            std::cout << ">> Await successful communication of all participants" << std::endl;
        }
        for (auto& p : syncParticipantsAutonomousA)
            p.AwaitCommunication();
        for (auto& p : syncParticipantsAutonomousB)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants)
            p.AwaitCommunication();

        if (verbose)
        {
            std::cout << ">> Hop off with autonomous B participants" << std::endl;
        }

        // Shutdown autonomous B participants
        for (auto& p : syncParticipantsAutonomousB)
            p.lifecycleService->Stop("Hop Off");
        JoinParticipantThreads(participantThreads_Sync_AutonomousB);

        // Reset communication to repeat the cycle
        expectedReceptions = syncParticipantsAutonomousA.size() + asyncParticipants.size();
        for (auto& p : syncParticipantsAutonomousA)
            p.ResetReception();
        for (auto& p : syncParticipantsAutonomousB)
            p.ResetReception();
        for (auto& p : asyncParticipants)
            p.ResetReception();
    }

    // Shutdown async participants
    for (auto& p : asyncParticipants)
        p.i.runAsync = false;
    JoinParticipantThreads(participantThreads_Async_Invalid);

    // Shutdown autonomous A participants
    for (auto& p : syncParticipantsAutonomousA)
        p.lifecycleService->Stop("End Test Off");
    JoinParticipantThreads(participantThreads_Sync_AutonomousA);

    StopRegistry();
}

} // anonymous namespace
