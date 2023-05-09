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

#include "GetTestPid.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/pubsub/PubSubSpec.hpp"
#include "silkit/services/orchestration/string_utils.hpp"

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
std::chrono::milliseconds communicationTimeout{10000ms};
std::chrono::milliseconds asyncDelayBetweenPublication{50ms};

enum class TimeMode
{
    Async,
    Sync
};

class ITest_ParticipantModes : public testing::Test
{
protected:
    ITest_ParticipantModes() {}

    struct TestParticipant
    {
        TestParticipant(const std::string& newName, TimeMode newTimeMode, OperationMode newOperationMode)
        {
            static size_t globalParticipantIndex = 0;

            name = newName;
            id = static_cast<uint8_t>(globalParticipantIndex++);
            timeMode = newTimeMode;
            lifeCycleOperationMode = newOperationMode;
        }

        TestParticipant(TestParticipant&&) = default;
        TestParticipant& operator=(TestParticipant&&) = default;

        struct ImmovableMembers
        {
            ImmovableMembers() = default;

            ImmovableMembers(ImmovableMembers&& other) noexcept
                : allReceived{other.allReceived.load()}
                , runAsync{other.runAsync.load()}
            {
            }

            ImmovableMembers& operator=(ImmovableMembers&& other) noexcept
            {
                if (this != &other)
                {
                    allReceived = other.allReceived.load();
                    runAsync = other.runAsync.load();
                }

                return *this;
            }

            std::atomic<bool> allReceived{false};
            std::atomic<bool> runAsync{true};
            std::atomic<bool> stopRequested{false};
            std::mutex participantStateMx;
        };

        ImmovableMembers i{};

        std::string name;
        uint8_t id;
        std::unique_ptr<IParticipant> participant;
        IDataPublisher* publisher;
        IDataSubscriber* subscriber;
        std::set<uint8_t> receivedIds;
        std::promise<void> allReceivedPromise;

        bool simtimePassed{false};
        std::promise<void> simtimePassedPromise;

        std::promise<void> simTaskFinishedPromise;

        TimeMode timeMode;
        OperationMode lifeCycleOperationMode;
        ILifecycleService* lifecycleService{nullptr};
        ISystemMonitor* systemMonitor{nullptr};
        ParticipantState participantState{ParticipantState::Invalid};

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
                EXPECT_EQ(futureStatus, std::future_status::ready)
                    << "Test Failure: Awaiting test communication timed out";
            }
            EXPECT_EQ(futureStatus, std::future_status::ready) << "Test Failure: Awaiting test communication timed out";
        }

        void Stop()
        {
            if (lifeCycleOperationMode == OperationMode::Invalid)
            {
                // No lifecycle -> Exit the while loop
                i.runAsync = false;
            }
            else
            {
                ParticipantState participantStateCopy;
                {
                    std::unique_lock<decltype(i.participantStateMx)> lock(i.participantStateMx);
                    participantStateCopy = participantState;
                }

                if (participantStateCopy == ParticipantState::Running)
                {
                    // Exit the while loop for Async + lifecycle
                    i.runAsync = false;
                    lifecycleService->Stop("End Test");
                }
                else
                {
                    // Call lifecycle->Stop as soon as in running state
                    i.stopRequested = true;
                }
            }
        }
    };

    void SyncParticipantThread(TestParticipant& testParticipant)
    {
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> config;
        if (logging)
        {
            config = SilKit::Config::MakeParticipantConfigurationWithLoggingImpl(Services::Logging::Level::Debug);
        }
        else
        {
            config = SilKit::Config::MakeEmptyParticipantConfigurationImpl();
        }

        testParticipant.participant = SilKit::CreateParticipantImpl(config, testParticipant.name, registryUri);

        testParticipant.systemMonitor = testParticipant.participant->CreateSystemMonitor();

        testParticipant.lifecycleService =
            testParticipant.participant->CreateLifecycleService({testParticipant.lifeCycleOperationMode});

        testParticipant.systemMonitor->AddParticipantStatusHandler(
            [&testParticipant](ParticipantStatus participantStatus) {
                if (participantStatus.participantName == testParticipant.name)
                {
                    //std::cout << "[" << participantStatus.participantName << "] state=" << participantStatus.state << std::endl;

                    {
                        std::unique_lock<decltype(testParticipant.i.participantStateMx)> lock(
                            testParticipant.i.participantStateMx);
                        testParticipant.participantState = participantStatus.state;
                    }
                    if (testParticipant.i.stopRequested && participantStatus.state == ParticipantState::Running)
                    {
                        testParticipant.lifecycleService->Stop("End Test");
                    }
                }
            });

        auto* timeSyncService = testParticipant.lifecycleService->CreateTimeSyncService();
        auto* logger = testParticipant.participant->GetLogger();

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

        timeSyncService->SetSimulationStepHandler(
            [logger, &testParticipant, this](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                testParticipant.publisher->Publish(std::vector<uint8_t>{testParticipant.id});
                std::stringstream ss;
                ss << "now=" << now.count() / 1e9 << "s";
                logger->Info(ss.str());
                if (!testParticipant.simtimePassed && now > simtimeToPass)
                {
                    testParticipant.simtimePassed = true;
                    testParticipant.simtimePassedPromise.set_value();
                }
            },
            1s);
        auto finalStateFuture = testParticipant.lifecycleService->StartLifecycle();
        finalStateFuture.get();

        testParticipant.participant.reset();
    }

    void AsyncParticipantThread(TestParticipant& testParticipant)
    {
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> config;
        if (logging)
        {
            config = SilKit::Config::MakeParticipantConfigurationWithLoggingImpl(Services::Logging::Level::Debug);
        }
        else
        {
            config = SilKit::Config::MakeEmptyParticipantConfigurationImpl();
        }

        testParticipant.participant = SilKit::CreateParticipantImpl(config, testParticipant.name, registryUri);

        testParticipant.systemMonitor = testParticipant.participant->CreateSystemMonitor();

        testParticipant.lifecycleService =
            testParticipant.participant->CreateLifecycleService({testParticipant.lifeCycleOperationMode});

        testParticipant.systemMonitor->AddParticipantStatusHandler(
            [&testParticipant](ParticipantStatus participantStatus) {
                if (participantStatus.participantName == testParticipant.name)
                {
                    //std::cout << "[" << participantStatus.participantName << "] state=" << participantStatus.state << std::endl;

                    {
                        std::unique_lock<decltype(testParticipant.i.participantStateMx)> lock(
                            testParticipant.i.participantStateMx);
                        testParticipant.participantState = participantStatus.state;
                    }

                    if (testParticipant.i.stopRequested && participantStatus.state == ParticipantState::Running)
                    {
                        testParticipant.i.runAsync = false;
                        testParticipant.lifecycleService->Stop("End Test");
                    }
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
            while (testParticipant.i.runAsync)
            {
                testParticipant.publisher->Publish(std::vector<uint8_t>{testParticipant.id});
                std::this_thread::sleep_for(asyncDelayBetweenPublication);
            }
            testParticipant.simTaskFinishedPromise.set_value();
        };

        if (testParticipant.lifeCycleOperationMode != OperationMode::Invalid)
        {
            auto finalStateFuture = testParticipant.lifecycleService->StartLifecycle();

            std::thread runTaskThread{runTask};
            runTaskThread.detach();

            auto simTaskFinishedFuture = testParticipant.simTaskFinishedPromise.get_future();
            simTaskFinishedFuture.get();

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

        // Explicitly delete the com adapter to end the async participant
        testParticipant.participant.reset();
    }

    struct SystemControllerParticipant
    {
        SilKit::Experimental::Services::Orchestration::ISystemController* systemController;
        ISystemMonitor* systemMonitor;
        ILifecycleService* lifecycleService;
        std::future<ParticipantState> finalState;
        std::atomic<bool> stopRequested{false};
        ParticipantState participantState{ParticipantState::Invalid};
        std::mutex participantStateMx;

        void Stop()
        {
            ParticipantState participantStateCopy;
            {
                std::unique_lock<decltype(participantStateMx)> lock(participantStateMx);
                participantStateCopy = participantState;
            }

            if (participantStateCopy == ParticipantState::Running)
            {
                lifecycleService->Stop("End Test");
            }
            else
            {
                // Call lifecycle->Stop as soon as in running state
                stopRequested = true;
            }
        }
    };

    void SystemStateHandler(SystemState newState)
    {
        switch (newState)
        {
        case SystemState::Error:
            std::cout << "SystemState::Error -> Aborting simulation" << std ::endl;
            systemControllerParticipant.systemController->AbortSimulation();
            break;

        case SystemState::Running: break;

        default: break;
        }
    }

    void SystemControllerParticipantThread(const std::vector<std::string>& required)
    {
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> config;
        config = SilKit::Config::MakeEmptyParticipantConfigurationImpl();

        auto participant = SilKit::CreateParticipantImpl(config, systemControllerParticipantName, registryUri);

        auto participantInternal = dynamic_cast<SilKit::Core::IParticipantInternal*>(participant.get());
        systemControllerParticipant.systemController = participantInternal->GetSystemController();

        systemControllerParticipant.systemMonitor = participant->CreateSystemMonitor();
        systemControllerParticipant.lifecycleService =
            participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});

        systemControllerParticipant.systemController->SetWorkflowConfiguration({required});

        systemControllerParticipant.systemMonitor->AddSystemStateHandler([this](SystemState newState) {
            //std::cout << "System state=" << newState << std::endl;

            SystemStateHandler(newState);
        });

        systemControllerParticipant.systemMonitor->AddParticipantStatusHandler(
            [this](ParticipantStatus participantStatus) {
                //std::cout << "[" << participantStatus.participantName << "] state=" << participantStatus.state
                //          << std::endl;

                if (participantStatus.participantName == systemControllerParticipantName)
                {
                    //std::cout << "[" << participantStatus.participantName << "] state=" << participantStatus.state << std::endl;

                    {
                        std::unique_lock<decltype(systemControllerParticipant.participantStateMx)> lock(
                            systemControllerParticipant.participantStateMx);
                        systemControllerParticipant.participantState = participantStatus.state;
                    }

                    if (systemControllerParticipant.stopRequested
                        && participantStatus.state == ParticipantState::Running)
                    {
                        systemControllerParticipant.lifecycleService->Stop("End Test");
                    }
                }
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
        systemControllerParticipant.Stop();
        participantThread_SystemController.shutdownFuture.get();
        participantThread_SystemController.thread.join();

        systemControllerParticipant.systemController = nullptr;
        systemControllerParticipant.systemMonitor = nullptr;
        systemControllerParticipant.lifecycleService = nullptr;
        systemControllerParticipant.stopRequested = false;
        systemControllerParticipant.participantState = ParticipantState::Invalid;
    }

    void RunRegistry()
    {
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> config;
        config = SilKit::Config::MakeEmptyParticipantConfiguration();
        registry = SilKit::Vendor::Vector::CreateSilKitRegistry(config);
        registry->StartListening(registryUri);
    }

    void StopRegistry() { registry.reset(); }

    void RunParticipants(std::vector<TestParticipant>& participants)
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
                    participantThreads_Sync_Autonomous.emplace_back([this, &p] {
                        SyncParticipantThread(p);
                    });
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

    void SetUp() override { registryUri = MakeTestRegistryUri(); }

protected:
    std::vector<std::string> requiredParticipantNames{};
    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> registry;

    SystemControllerParticipant systemControllerParticipant;
    ParticipantThread participantThread_SystemController;

    std::vector<ParticipantThread> participantThreads_Sync_Invalid;
    std::vector<ParticipantThread> participantThreads_Sync_Autonomous;
    std::vector<ParticipantThread> participantThreads_Sync_Coordinated;

    std::vector<ParticipantThread> participantThreads_Async_Invalid;
    std::vector<ParticipantThread> participantThreads_Async_Autonomous;
    std::vector<ParticipantThread> participantThreads_Async_Coordinated;

    std::chrono::seconds simtimeToPass{3s};
    bool runAsync{true};

    const bool verbose = true;
    const bool logging = false;

    std::string registryUri;
};

// ---------------------------------
// Single flavour participant pairs:
// ---------------------------------

// Time     Lifecycle       Required
// ---------------------------------
// Async    Invalid         NonReq
// Async    Autonomous      NonReq
// Async    Coordinated     Req
// Sync     Invalid         NonReq
// Sync     Autonomous      NonReq
// Sync     Coordinated     Req

TEST_F(ITest_ParticipantModes, test_Async_Invalid_NonReq)
{
    RunRegistry();

    // Participants
    std::vector<TestParticipant> testParticipants;
    testParticipants.push_back({"AsyncInvalid1", TimeMode::Async, OperationMode::Invalid});
    testParticipants.push_back({"AsyncInvalid2", TimeMode::Async, OperationMode::Invalid});
    testParticipants.push_back({"AsyncInvalid3", TimeMode::Async, OperationMode::Invalid});
    testParticipants.push_back({"AsyncInvalid4", TimeMode::Async, OperationMode::Invalid});
    expectedReceptions = testParticipants.size();

    // Start
    RunParticipants(testParticipants);

    // Await successful communication
    for (auto& p : testParticipants)
        p.AwaitCommunication();

    // Async: Stop task
    for (auto& p : testParticipants)
        p.i.runAsync = false;

    // Shutdown
    JoinParticipantThreads(participantThreads_Async_Invalid);
    StopRegistry();
}

TEST_F(ITest_ParticipantModes, test_Async_Autonomous_NonReq)
{
    RunRegistry();

    // Participants
    std::vector<TestParticipant> testParticipants;
    testParticipants.push_back({"AsyncAutonomous1", TimeMode::Async, OperationMode::Autonomous});
    testParticipants.push_back({"AsyncAutonomous2", TimeMode::Async, OperationMode::Autonomous});
    testParticipants.push_back({"AsyncAutonomous3", TimeMode::Async, OperationMode::Autonomous});
    testParticipants.push_back({"AsyncAutonomous4", TimeMode::Async, OperationMode::Autonomous});
    expectedReceptions = testParticipants.size();

    // Start
    RunParticipants(testParticipants);

    // Await successful communication
    for (auto& p : testParticipants)
        p.AwaitCommunication();

    // Stop
    for (auto& p : testParticipants)
        p.Stop();

    // Shutdown
    JoinParticipantThreads(participantThreads_Async_Autonomous);

    StopRegistry();
}

TEST_F(ITest_ParticipantModes, test_Async_Coordinated_Req)
{
    RunRegistry();

    // Participants
    std::vector<TestParticipant> testParticipants;
    testParticipants.push_back({"AsyncCoordinated1", TimeMode::Async, OperationMode::Coordinated});
    testParticipants.push_back({"AsyncCoordinated2", TimeMode::Async, OperationMode::Coordinated});
    testParticipants.push_back({"AsyncCoordinated3", TimeMode::Async, OperationMode::Coordinated});
    testParticipants.push_back({"AsyncCoordinated4", TimeMode::Async, OperationMode::Coordinated});
    expectedReceptions = testParticipants.size();

    // Coordinated are required
    std::vector<std::string> required{};
    for (auto&& p : testParticipants)
    {
        if (p.lifeCycleOperationMode == OperationMode::Coordinated)
        {
            required.push_back(p.name);
        }
    }
    RunSystemController(required);

    // Start
    RunParticipants(testParticipants);

    // Await successful communication
    for (auto& p : testParticipants)
        p.AwaitCommunication();

    // Async: Stop task
    for (auto& p : testParticipants)
        p.i.runAsync = false;

    // Stop Coordinated Lifecylce: One participant can stop all
    testParticipants[0].Stop();

    // Shutdown
    JoinParticipantThreads(participantThreads_Async_Coordinated);
    StopSystemController();
    StopRegistry();
}

// Scenario conceptually impossible -> throw ConfigurationError
TEST_F(ITest_ParticipantModes, test_Sync_Invalid_NonReq)
{
    RunRegistry();

    // Participants
    std::vector<TestParticipant> testParticipants;
    testParticipants.push_back({"SyncInvalid1", TimeMode::Sync, OperationMode::Invalid});
    expectedReceptions = testParticipants.size();

    RunParticipants(testParticipants);
    EXPECT_THROW(JoinParticipantThreads(participantThreads_Sync_Invalid), SilKit::ConfigurationError);

    StopRegistry();
}

TEST_F(ITest_ParticipantModes, DISABLED_test_Sync_Autonomous_NonReq)
{
    RunRegistry();

    // Participants
    std::vector<TestParticipant> testParticipants;
    testParticipants.push_back({"SyncAutonomous1", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous2", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous3", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous4", TimeMode::Sync, OperationMode::Autonomous});
    expectedReceptions = testParticipants.size();

    // Start
    RunParticipants(testParticipants);

    // Await successful communication
    for (auto& p : testParticipants)
        p.AwaitCommunication();

    // Stop Lifecylce
    for (auto& p : testParticipants)
        p.Stop();

    // Shutdown
    JoinParticipantThreads(participantThreads_Sync_Autonomous);

    StopRegistry();
}

TEST_F(ITest_ParticipantModes, test_Sync_Coordinated_Req)
{
    RunRegistry();

    // Participants
    std::vector<TestParticipant> testParticipants;
    testParticipants.push_back({"SyncCoordinated1", TimeMode::Sync, OperationMode::Coordinated});
    testParticipants.push_back({"SyncCoordinated2", TimeMode::Sync, OperationMode::Coordinated});
    testParticipants.push_back({"SyncCoordinated3", TimeMode::Sync, OperationMode::Coordinated});
    testParticipants.push_back({"SyncCoordinated4", TimeMode::Sync, OperationMode::Coordinated});
    expectedReceptions = testParticipants.size();

    // Coordinated are required
    std::vector<std::string> required{};
    for (auto&& p : testParticipants)
    {
        if (p.lifeCycleOperationMode == OperationMode::Coordinated)
        {
            required.push_back(p.name);
        }
    }
    RunSystemController(required);

    // Start
    RunParticipants(testParticipants);

    // Await successful communication
    for (auto& p : testParticipants)
        p.AwaitCommunication();

    // Stop Lifecylce
    for (auto& p : testParticipants)
        p.Stop();

    // Shutdown
    JoinParticipantThreads(participantThreads_Sync_Coordinated);

    StopSystemController();
    StopRegistry();
}

TEST_F(ITest_ParticipantModes, DISABLED_test_P1AsyncInvalid_P2SyncAutonomous)
{
    RunRegistry();

    // Participants
    std::vector<TestParticipant> testParticipants;
    testParticipants.push_back({"ASyncInvalid1", TimeMode::Async, OperationMode::Invalid});
    testParticipants.push_back({"ASyncInvalid2", TimeMode::Async, OperationMode::Invalid});
    testParticipants.push_back({"ASyncInvalid3", TimeMode::Async, OperationMode::Invalid});
    testParticipants.push_back({"ASyncInvalid4", TimeMode::Async, OperationMode::Invalid});
    testParticipants.push_back({"SyncAutonomous1", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous2", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous3", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous4", TimeMode::Sync, OperationMode::Autonomous});
    expectedReceptions = testParticipants.size();

    // Start
    RunParticipants(testParticipants);

    // Await successful communication
    for (auto& p : testParticipants)
        p.AwaitCommunication();

    // Stop
    for (auto& p : testParticipants)
    {
        p.Stop();
    }

    // Shutdown
    JoinParticipantThreads(participantThreads_Async_Invalid);
    JoinParticipantThreads(participantThreads_Sync_Autonomous);

    StopRegistry();
}

TEST_F(ITest_ParticipantModes, DISABLED_test_Sync_P1Coordinated_P2Autonomous)
{
    RunRegistry();

    // Participants
    std::vector<TestParticipant> testParticipants;
    testParticipants.push_back({"SyncCoordinated1", TimeMode::Sync, OperationMode::Coordinated});
    testParticipants.push_back({"SyncCoordinated2", TimeMode::Sync, OperationMode::Coordinated});
    testParticipants.push_back({"SyncCoordinated3", TimeMode::Sync, OperationMode::Coordinated});
    testParticipants.push_back({"SyncAutonomous1", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous2", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous3", TimeMode::Sync, OperationMode::Autonomous});
    expectedReceptions = testParticipants.size();

    // Coordinated are required
    std::vector<std::string> required{};
    for (auto&& p : testParticipants)
    {
        if (p.lifeCycleOperationMode == OperationMode::Coordinated)
        {
            required.push_back(p.name);
        }
    }
    RunSystemController(required);

    // Start
    RunParticipants(testParticipants);

    // Await successful communication
    for (auto& p : testParticipants)
        p.AwaitCommunication();

    // Stop Coordinated Lifecylce: One participant can stop all
    for (auto& p : testParticipants)
        p.Stop();

    // Shutdown
    JoinParticipantThreads(participantThreads_Sync_Coordinated);

    StopSystemController();
    StopRegistry();
}

TEST_F(ITest_ParticipantModes, DISABLED_test_Sync_P1Autonomous_P2Autonomous)
{
    RunRegistry();

    // Participants
    std::vector<TestParticipant> testParticipants;
    testParticipants.push_back({"SyncAutonomous1", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous2", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous3", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous4", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous5", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous6", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous7", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous8", TimeMode::Sync, OperationMode::Autonomous});
    expectedReceptions = testParticipants.size();

    // Start
    RunParticipants(testParticipants);

    // Await successful communication
    for (auto& p : testParticipants)
        p.AwaitCommunication();

    // Stop
    for (auto& p : testParticipants)
        p.Stop();

    // Shutdown
    JoinParticipantThreads(participantThreads_Sync_Coordinated);

    StopRegistry();
}

// ----------------
// All combinations
// ----------------

TEST_F(ITest_ParticipantModes, DISABLED_test_Combinations)
{
    RunRegistry();

    std::vector<TimeMode> timeModes = {TimeMode::Async, TimeMode::Sync};
    std::vector<OperationMode> operationModes = {OperationMode::Invalid, OperationMode::Autonomous,
                                                 OperationMode::Coordinated};

    std::map<TimeMode, std::string> timeModeNames = {{TimeMode::Async, "Async"}, {TimeMode::Sync, "Sync"}};
    std::map<OperationMode, std::string> operationModesNames = {{OperationMode::Invalid, "Invalid"},
                                                                {OperationMode::Autonomous, "Autonomous"},
                                                                {OperationMode::Coordinated, "Coordinated"}};

    // These loops test (all but the first) combinations twice.
    // This is intended as the order of which participant is started first is then tested as well.
    for (auto p1_timeMode : timeModes)
    {
        for (auto p1_operationMode : operationModes)
        {
            for (auto p2_timeMode : timeModes)
            {
                for (auto p2_operationMode : operationModes)
                {
                    // Sync needs lifecycle, skip
                    if ((p1_timeMode == TimeMode::Sync && p1_operationMode == OperationMode::Invalid)
                        || (p2_timeMode == TimeMode::Sync && p2_operationMode == OperationMode::Invalid))
                    {
                        continue;
                    }

                    if (verbose)
                    {
                        std::cout << "P1:" << timeModeNames[p1_timeMode] << " + "
                                  << operationModesNames[p1_operationMode] << " P2:" << timeModeNames[p2_timeMode]
                                  << " + " << operationModesNames[p2_operationMode];
                    }

                    // Participants
                    std::vector<TestParticipant> testParticipants;
                    testParticipants.push_back({"P1A", p1_timeMode, p1_operationMode});
                    testParticipants.push_back({"P1B", p1_timeMode, p1_operationMode});
                    testParticipants.push_back({"P1C", p1_timeMode, p1_operationMode});
                    testParticipants.push_back({"P1D", p1_timeMode, p1_operationMode});
                    testParticipants.push_back({"P2A", p2_timeMode, p2_operationMode});
                    testParticipants.push_back({"P2B", p2_timeMode, p2_operationMode});
                    testParticipants.push_back({"P2C", p2_timeMode, p2_operationMode});
                    testParticipants.push_back({"P2D", p2_timeMode, p2_operationMode});
                    expectedReceptions = testParticipants.size();

                    // Required
                    std::vector<std::string> required{};
                    if (p1_operationMode == OperationMode::Coordinated)
                    {
                        required.push_back("P1A");
                        required.push_back("P1B");
                        required.push_back("P1C");
                        required.push_back("P1D");
                    }
                    if (p2_operationMode == OperationMode::Coordinated)
                    {
                        required.push_back("P2A");
                        required.push_back("P2B");
                        required.push_back("P2C");
                        required.push_back("P2D");
                    }
                    if (!required.empty())
                    {
                        RunSystemController(required);
                    }

                    if (verbose)
                    {
                        std::cout << " -> Run participants";
                    }

                    // Start
                    RunParticipants(testParticipants);

                    if (verbose)
                    {
                        std::cout << " -> Await communication";
                    }

                    // Await successful communication
                    for (auto& p : testParticipants)
                    {
                        p.AwaitCommunication();
                    }

                    if (verbose)
                    {
                        std::cout << " -> Request Stop";
                    }

                    // Stop
                    for (auto& p : testParticipants)
                    {
                        p.Stop();
                    }

                    if (verbose)
                    {
                        std::cout << " -> Shutdown";
                    }

                    // Shutdown
                    if (!participantThreads_Sync_Invalid.empty())
                    {
                        JoinParticipantThreads(participantThreads_Sync_Invalid);
                    }
                    if (!participantThreads_Sync_Autonomous.empty())
                    {
                        JoinParticipantThreads(participantThreads_Sync_Autonomous);
                    }
                    if (!participantThreads_Sync_Coordinated.empty())
                    {
                        JoinParticipantThreads(participantThreads_Sync_Coordinated);
                    }
                    if (!participantThreads_Async_Invalid.empty())
                    {
                        JoinParticipantThreads(participantThreads_Async_Invalid);
                    }
                    if (!participantThreads_Async_Autonomous.empty())
                    {
                        JoinParticipantThreads(participantThreads_Async_Autonomous);
                    }
                    if (!participantThreads_Async_Coordinated.empty())
                    {
                        JoinParticipantThreads(participantThreads_Async_Coordinated);
                    }
                    if (!required.empty())
                    {
                        StopSystemController();
                    }

                    if (verbose)
                    {
                        std::cout << " -> Done" << std::endl;
                    }
                }
            }
        }
    }
    StopRegistry();
}

} // anonymous namespace
