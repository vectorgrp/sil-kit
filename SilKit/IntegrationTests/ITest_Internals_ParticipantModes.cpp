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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/pubsub/PubSubSpec.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"

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

std::atomic<bool> abortSystemControllerRequested{false};

static size_t expectedReceptions = 0;
static size_t globalParticipantIndex = 0;

std::chrono::milliseconds communicationTimeout{10000ms};
std::chrono::milliseconds asyncDelayBetweenPublication{50ms};

enum class TimeMode
{
    Async,
    Sync
};

class ITest_Internals_ParticipantModes : public testing::Test
{
protected:
    ITest_Internals_ParticipantModes() {}

    struct TestParticipant
    {
        TestParticipant(const std::string& newName, TimeMode newTimeMode, OperationMode newOperationMode)
        {
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
                , stopRequested{other.stopRequested.load()}
                , errorStateReached{other.errorStateReached.load()}
                , runningStateReached{other.runningStateReached.load()}
            {
            }

            ImmovableMembers& operator=(ImmovableMembers&& other) noexcept
            {
                if (this != &other)
                {
                    allReceived = other.allReceived.load();
                    stopRequested = other.stopRequested.load();
                    errorStateReached = other.errorStateReached.load();
                    runningStateReached = other.runningStateReached.load();
                }

                return *this;
            }

            std::atomic<bool> allReceived{false};
            std::atomic<bool> stopRequested{false};
            std::atomic<bool> errorStateReached{false};
            std::atomic<bool> runningStateReached{false};
        };

        ImmovableMembers i{};

        std::string name;
        uint8_t id;
        std::set<uint8_t> receivedIds;
        std::promise<void> allReceivedPromise;

        std::promise<void> errorStatePromise;
        std::promise<void> runningStatePromise;

        bool simtimePassed{false};
        std::promise<void> simtimePassedPromise;

        std::promise<void> simTaskFinishedPromise;

        bool allowInvalidLifeCycleOperationMode{false};

        TimeMode timeMode;
        OperationMode lifeCycleOperationMode;
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
                FAIL() << "Test Failure: Awaiting test communication timed out";
            }
        }

        void AwaitErrorState()
        {
            auto futureStatus = errorStatePromise.get_future().wait_for(communicationTimeout);
            if (futureStatus != std::future_status::ready)
            {
                FAIL() << "Test Failure: Awaiting error state timed out";
            }
        }

        void Stop()
        {
            i.stopRequested = true;
        }
    };

    void SyncParticipantThread(TestParticipant& testParticipant)
    {
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> config;
        if (logging)
        {
            config = SilKit::Config::MakeParticipantConfigurationWithLoggingImpl(logLevel);
        }
        else
        {
            config = SilKit::Config::MakeEmptyParticipantConfigurationImpl();
        }

        auto participant = SilKit::CreateParticipantImpl(config, testParticipant.name, _registryUri);
        auto* logger = participant->GetLogger();

        ILifecycleService* lifecycleService{};
        if (testParticipant.allowInvalidLifeCycleOperationMode
            || testParticipant.lifeCycleOperationMode != OperationMode::Invalid)
        {
            lifecycleService = participant->CreateLifecycleService({testParticipant.lifeCycleOperationMode});
        }

        auto participantInternal = dynamic_cast<SilKit::Core::IParticipantInternal*>(participant.get());
        auto systemController = participantInternal->GetSystemController();

        auto systemMonitor = participant->CreateSystemMonitor();
        systemMonitor->AddParticipantStatusHandler([&testParticipant, systemController, logger,
                                                    this](const Services::Orchestration::ParticipantStatus& status) {
            if (status.participantName == testParticipant.name)
            {
                if (status.state == ParticipantState::Error && !testParticipant.i.errorStateReached)
                {
                    testParticipant.i.errorStateReached = true;
                    testParticipant.errorStatePromise.set_value();

                    if (logging)
                    {
                        std::stringstream ss;
                        ss << "AbortSimulation due to ErrorState of participant \'" << testParticipant.name << "\'";
                        logger->Info(ss.str());
                    }

                    systemController->AbortSimulation();
                }
                else if (status.state == ParticipantState::Running && !testParticipant.i.runningStateReached)
                {
                    testParticipant.i.runningStateReached = true;
                    testParticipant.runningStatePromise.set_value();
                }
            }
        });


        ITimeSyncService* timeSyncService{};
        if (testParticipant.allowInvalidLifeCycleOperationMode
            || testParticipant.lifeCycleOperationMode != OperationMode::Invalid)
        {
            timeSyncService = lifecycleService->CreateTimeSyncService();
        }

        SilKit::Services::PubSub::PubSubSpec dataSpec{topic, mediaType};
        SilKit::Services::PubSub::PubSubSpec matchingDataSpec{topic, mediaType};
        auto publisher = participant->CreateDataPublisher("TestPublisher", dataSpec, 0);
        participant->CreateDataSubscriber(
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
            [lifecycleService, logger, &testParticipant, publisher, this](std::chrono::nanoseconds now,
                                                                          std::chrono::nanoseconds /*duration*/) {
            publisher->Publish(std::vector<uint8_t>{testParticipant.id});
            std::stringstream ss;
            ss << "now=" << now.count() / 1e9 << "s";
            logger->Info(ss.str());
            if (!testParticipant.simtimePassed && now > _simtimeToPass)
            {
                testParticipant.simtimePassed = true;
                testParticipant.simtimePassedPromise.set_value();
            }
            if (testParticipant.i.stopRequested)
            {
                testParticipant.i.stopRequested = false;
                lifecycleService->Stop("End Test");
            }
        },
            1s);

        if (testParticipant.lifeCycleOperationMode != OperationMode::Invalid)
        {
            auto finalStateFuture = lifecycleService->StartLifecycle();
            finalStateFuture.get();
        }
    }

    void AsyncParticipantThread(TestParticipant& testParticipant)
    {
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> config;
        if (logging)
        {
            config = SilKit::Config::MakeParticipantConfigurationWithLoggingImpl(logLevel);
        }
        else
        {
            config = SilKit::Config::MakeEmptyParticipantConfigurationImpl();
        }

        auto participant = SilKit::CreateParticipantImpl(config, testParticipant.name, _registryUri);
        auto* logger = participant->GetLogger();

        ILifecycleService* lifecycleService{};
        if (testParticipant.allowInvalidLifeCycleOperationMode
            || testParticipant.lifeCycleOperationMode != OperationMode::Invalid)
        {
            lifecycleService = participant->CreateLifecycleService({testParticipant.lifeCycleOperationMode});
        }

        auto participantInternal = dynamic_cast<SilKit::Core::IParticipantInternal*>(participant.get());
        auto systemController = participantInternal->GetSystemController();
        auto systemMonitor = participant->CreateSystemMonitor();
        systemMonitor->AddParticipantStatusHandler([&testParticipant, systemController, logger,
                                                    this](const Services::Orchestration::ParticipantStatus& status) {
            if (status.participantName == testParticipant.name)
            {
                if (status.state == ParticipantState::Error && !testParticipant.i.errorStateReached)
                {
                    // We also set the runningStatePromise to skip waiting for this
                    testParticipant.runningStatePromise.set_value();

                    testParticipant.i.errorStateReached = true;
                    testParticipant.errorStatePromise.set_value();
                    if (logging)
                    {
                        std::stringstream ss;
                        ss << "AbortSimulation due to ErrorState of participant \'" << testParticipant.name << "\'";
                        logger->Info(ss.str());
                    }
                    systemController->AbortSimulation();
                }
                else if (status.state == ParticipantState::Running && !testParticipant.i.runningStateReached)
                {
                    testParticipant.i.runningStateReached = true;
                    testParticipant.runningStatePromise.set_value();
                }
            }
        });

        SilKit::Services::PubSub::PubSubSpec dataSpec{topic, mediaType};
        SilKit::Services::PubSub::PubSubSpec matchingDataSpec{topic, mediaType};
        auto publisher = participant->CreateDataPublisher("TestPublisher", dataSpec, 0);
        participant->CreateDataSubscriber(
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

        auto runTask = [&testParticipant, publisher]() {
            while (!testParticipant.i.stopRequested)
            {
                publisher->Publish(std::vector<uint8_t>{testParticipant.id});
                std::this_thread::sleep_for(asyncDelayBetweenPublication);
            }
            testParticipant.simTaskFinishedPromise.set_value();
        };

        if (testParticipant.lifeCycleOperationMode != OperationMode::Invalid)
        {
            auto finalStateFuture = lifecycleService->StartLifecycle();

            if (!testParticipant.i.errorStateReached)
            {
                // Wait for ParticipantState::Running
                auto runningStateFutureStatus =
                    testParticipant.runningStatePromise.get_future().wait_for(communicationTimeout);
                if (runningStateFutureStatus != std::future_status::ready)
                {
                    FAIL() << "Test Failure: Awaiting running state timed out";
                }
            }

            // Run the task
            std::thread runTaskThread{runTask};
            runTaskThread.detach();

            // Wait for task to have received a stop request
            auto simTaskFinishedFuture = testParticipant.simTaskFinishedPromise.get_future();
            simTaskFinishedFuture.get();

            // Stop the lifecycle
            lifecycleService->Stop("End Test");

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
    }

    void SystemControllerParticipantThread(const std::vector<std::string>& required)
    {
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> config;
        if (logging)
        {
            config = SilKit::Config::MakeParticipantConfigurationWithLoggingImpl(logLevel);
        }
        else
        {
            config = SilKit::Config::MakeEmptyParticipantConfigurationImpl();
        }

        auto systemControllerParticipant =
            SilKit::CreateParticipantImpl(config, systemControllerParticipantName, _registryUri);

        auto participantInternal = dynamic_cast<SilKit::Core::IParticipantInternal*>(systemControllerParticipant.get());
        auto systemController = participantInternal->GetSystemController();

        auto* logger = systemControllerParticipant->GetLogger();

        auto systemMonitor = systemControllerParticipant->CreateSystemMonitor();

        systemMonitor->AddParticipantStatusHandler([this, logger](ParticipantStatus newStatus) {
            if (logging)
            {
                std::stringstream ss;
                ss << "New ParticipantState of " << newStatus.participantName << ": " << newStatus.state
                   << "; Reason: " << newStatus.enterReason;
                logger->Info(ss.str());
            }
        });

        systemMonitor->AddSystemStateHandler([this, logger, systemController](SystemState newState) {
            if (logging)
            {
                std::stringstream ss;
                ss << "New SystemState " << newState;
                logger->Info(ss.str());
            }
            switch (newState)
            {
            case SystemState::Error:
                if (verbose)
                {
                    std::cout << "SystemState::Error -> Aborting simulation" << std ::endl;
                }
                if (logging)
                {
                    logger->Info("Aborting simulation due to SystemState::Error");
                }
                systemController->AbortSimulation();
                break;

            case SystemState::Running:
                break;

            default:
                break;
            }
        });

        systemController->SetWorkflowConfiguration({required});

        ILifecycleService* systemControllerLifecycleService = systemControllerParticipant->CreateLifecycleService(
            {SilKit::Services::Orchestration::OperationMode::Coordinated});

        auto finalState = systemControllerLifecycleService->StartLifecycle();

        std::promise<void> abortThreadDone{};
        auto waitForAbortTask = [&abortThreadDone, systemController, logger, this]() {
            while (!abortSystemControllerRequested)
            {
                std::this_thread::sleep_for(1ms);
            }

            if (logging)
            {
                logger->Info("AbortSimulation requested");
            }
            systemController->AbortSimulation();
            abortThreadDone.set_value();
        };
        std::thread abortThread{waitForAbortTask};
        abortThread.detach();
        abortThreadDone.get_future().get();
        if (abortThread.joinable())
        {
            abortThread.join();
        }
        abortSystemControllerRequested = false;

        finalState.get();
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

            participantThread_SystemController =
                ParticipantThread{[this, required] { SystemControllerParticipantThread(required); }};
        }
    }

    void AbortSystemController()
    {
        abortSystemControllerRequested = true;
        participantThread_SystemController.shutdownFuture.get();
        if (participantThread_SystemController.thread.joinable())
        {
            participantThread_SystemController.thread.join();
        }
    }

    void RunRegistry()
    {
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> config;
        config = SilKit::Config::MakeEmptyParticipantConfiguration();
        registry = SilKit::Vendor::Vector::CreateSilKitRegistry(config);
        _registryUri = registry->StartListening("silkit://localhost:0");
    }

    void StopRegistry()
    {
        registry.reset();
    }

    void RunParticipants(std::list<TestParticipant>& participants)
    {
        for (auto& p : participants)
        {
            if (p.timeMode == TimeMode::Async)
            {
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
                    participantThreads_Sync_Autonomous.emplace_back([this, &p] { SyncParticipantThread(p); });
                }
                else if (p.lifeCycleOperationMode == OperationMode::Coordinated)
                {
                    _participantThreads_Sync_Coordinated.emplace_back([this, &p] { SyncParticipantThread(p); });
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

    void SetUp() override
    {
        globalParticipantIndex = 0;
    }

protected:
    std::vector<std::string> requiredParticipantNames{};
    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> registry;

    ParticipantThread participantThread_SystemController;

    std::vector<ParticipantThread> _participantThreads_Sync_Invalid;
    std::vector<ParticipantThread> participantThreads_Sync_Autonomous;
    std::vector<ParticipantThread> _participantThreads_Sync_Coordinated;

    std::vector<ParticipantThread> _participantThreads_Async_Invalid;
    std::vector<ParticipantThread> _participantThreads_Async_Autonomous;
    std::vector<ParticipantThread> _participantThreads_Async_Coordinated;

    std::chrono::seconds _simtimeToPass{3s};

    const bool verbose = true;
    const bool logging = false;
    const Services::Logging::Level logLevel = Services::Logging::Level::Trace;

    std::string _registryUri{"not yet defined"};

};

// --------------------------
// Disallowed modes

// Time     Lifecycle       Required
// ---------------------------------
// Async    Coordinated     NonReq      -> Disallowed: Coordinated must be required
// Sync     Coordinated     NonReq      -> Disallowed: Coordinated must be required
// Sync     Invalid         Req/NonReq  -> Disallowed

TEST_F(ITest_Internals_ParticipantModes, test_AsyncCoordinatedNonReq_disallowed)
{
    RunRegistry();

    // Participants
    std::list<TestParticipant> testParticipants;
    testParticipants.push_back({"AsyncCoordinated1", TimeMode::Async, OperationMode::Coordinated});

    // Workflow configuration without "AsyncCoordinated1"
    RunSystemController({"NoSuchParticipant"});

    RunParticipants(testParticipants);

    // Await error state
    for (auto& p : testParticipants)
        p.AwaitErrorState();

    // Stop to exit the async task
    for (auto& p : testParticipants)
        p.Stop();

    // Shutdown
    JoinParticipantThreads(_participantThreads_Async_Coordinated);
    AbortSystemController();
    StopRegistry();
}

TEST_F(ITest_Internals_ParticipantModes, test_SyncCoordinatedNonReq_disallowed)
{
    RunRegistry();

    // Participants
    std::list<TestParticipant> testParticipants;
    testParticipants.push_back({"SyncCoordinated1", TimeMode::Sync, OperationMode::Coordinated});

    // Workflow configuration without "AsyncCoordinated1"
    RunSystemController({"NoSuchParticipant"});

    RunParticipants(testParticipants);

    // Await error state
    for (auto& p : testParticipants)
        p.AwaitErrorState();

    // Stop to exit the async task
    for (auto& p : testParticipants)
        p.Stop();

    // Shutdown
    JoinParticipantThreads(_participantThreads_Sync_Coordinated);
    AbortSystemController();
    StopRegistry();
}

TEST_F(ITest_Internals_ParticipantModes, test_SyncInvalid_disallowed)
{
    RunRegistry();

    // Participants
    std::list<TestParticipant> testParticipants;
    testParticipants.push_back({"SyncInvalid1", TimeMode::Sync, OperationMode::Invalid});
    testParticipants.front().allowInvalidLifeCycleOperationMode = true;

    RunParticipants(testParticipants);
    // Cannot create lifecycle service with OperationMode::Invalid
    EXPECT_THROW(JoinParticipantThreads(_participantThreads_Sync_Invalid), SilKit::ConfigurationError);

    StopRegistry();
}


// --------------------------
// Single participant flavors

// Time     Lifecycle       Required
// ---------------------------------
// Async    Invalid         NonReq
// Async    Autonomous      NonReq
// Async    Autonomous      Req
// Async    Coordinated     NonReq      -> Disallowed
// Async    Coordinated     Req

// Sync     Invalid         Req/NonReq  -> Disallowed
// Sync     Autonomous      NonReq
// Sync     Autonomous      Req
// Sync     Coordinated     Req
// Sync     Coordinated     NonReq      -> Disallowed

TEST_F(ITest_Internals_ParticipantModes, test_AsyncInvalidNonReq)
{
    RunRegistry();

    // Participants
    std::list<TestParticipant> testParticipants;
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
        p.Stop();

    // Shutdown
    JoinParticipantThreads(_participantThreads_Async_Invalid);
    StopRegistry();
}

TEST_F(ITest_Internals_ParticipantModes, test_AsyncAutonomousNonReq)
{
    RunRegistry();

    // Participants
    std::list<TestParticipant> testParticipants;
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
    JoinParticipantThreads(_participantThreads_Async_Autonomous);

    StopRegistry();
}

TEST_F(ITest_Internals_ParticipantModes, test_AsyncAutonomousReq)
{
    RunRegistry();

    // Participants
    std::list<TestParticipant> testParticipants;
    testParticipants.push_back({"AsyncAutonomous1", TimeMode::Async, OperationMode::Autonomous});
    testParticipants.push_back({"AsyncAutonomous2", TimeMode::Async, OperationMode::Autonomous});
    testParticipants.push_back({"AsyncAutonomous3", TimeMode::Async, OperationMode::Autonomous});
    testParticipants.push_back({"AsyncAutonomous4", TimeMode::Async, OperationMode::Autonomous});
    expectedReceptions = testParticipants.size();

    // Autonomous are required here, but have no effect since no coordinated participants are in the mix
    std::vector<std::string> required{};
    for (auto&& p : testParticipants)
    {
        required.push_back(p.name);
    }
    RunSystemController(required);

    // Start
    RunParticipants(testParticipants);

    // Await successful communication
    for (auto& p : testParticipants)
        p.AwaitCommunication();

    // Stop
    for (auto& p : testParticipants)
        p.Stop();

    // Shutdown
    JoinParticipantThreads(_participantThreads_Async_Autonomous);
    AbortSystemController();
    StopRegistry();
}

TEST_F(ITest_Internals_ParticipantModes, test_AsyncCoordinatedReq)
{
    RunRegistry();

    // Participants
    std::list<TestParticipant> testParticipants;
    testParticipants.push_back({"AsyncCoordinated1", TimeMode::Async, OperationMode::Coordinated});
    testParticipants.push_back({"AsyncCoordinated2", TimeMode::Async, OperationMode::Coordinated});
    testParticipants.push_back({"AsyncCoordinated3", TimeMode::Async, OperationMode::Coordinated});
    testParticipants.push_back({"AsyncCoordinated4", TimeMode::Async, OperationMode::Coordinated});
    expectedReceptions = testParticipants.size();

    // Coordinated are required
    std::vector<std::string> required{};
    for (auto&& p : testParticipants)
    {
        required.push_back(p.name);
    }
    RunSystemController(required);

    // Start
    RunParticipants(testParticipants);

    // Await successful communication
    for (auto& p : testParticipants)
        p.AwaitCommunication();

    // Async: Stop task
    for (auto& p : testParticipants)
        p.Stop();

    // Stop Coordinated Lifecylce: One participant can stop all
    testParticipants.front().Stop();

    // Shutdown
    JoinParticipantThreads(_participantThreads_Async_Coordinated);

    AbortSystemController();
    StopRegistry();
}


TEST_F(ITest_Internals_ParticipantModes, test_SyncAutonomousNonReq)
{
    RunRegistry();

    // Participants
    std::list<TestParticipant> testParticipants;
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
        p.Stop();

    // Shutdown
    JoinParticipantThreads(participantThreads_Sync_Autonomous);

    StopRegistry();
}


TEST_F(ITest_Internals_ParticipantModes, test_SyncAutonomousReq)
{
    RunRegistry();

    // Participants
    std::list<TestParticipant> testParticipants;
    testParticipants.push_back({"SyncAutonomous1", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous2", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous3", TimeMode::Sync, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous4", TimeMode::Sync, OperationMode::Autonomous});
    expectedReceptions = testParticipants.size();

    // Autonomous are required here, but have no effect since no coordinated participants are in the mix
    std::vector<std::string> required{};
    for (auto&& p : testParticipants)
    {
        required.push_back(p.name);
    }
    RunSystemController(required);

    // Start
    RunParticipants(testParticipants);

    // Await successful communication
    for (auto& p : testParticipants)
        p.AwaitCommunication();

    // Stop
    for (auto& p : testParticipants)
        p.Stop();

    // Shutdown
    JoinParticipantThreads(participantThreads_Sync_Autonomous);

    AbortSystemController();
    StopRegistry();
}


TEST_F(ITest_Internals_ParticipantModes, test_SyncCoordinatedReq)
{
    RunRegistry();

    // Participants
    std::list<TestParticipant> testParticipants;
    testParticipants.push_back({"SyncCoordinated1", TimeMode::Sync, OperationMode::Coordinated});
    testParticipants.push_back({"SyncCoordinated2", TimeMode::Sync, OperationMode::Coordinated});
    testParticipants.push_back({"SyncCoordinated3", TimeMode::Sync, OperationMode::Coordinated});
    testParticipants.push_back({"SyncCoordinated4", TimeMode::Sync, OperationMode::Coordinated});
    expectedReceptions = testParticipants.size();

    // Coordinated are required
    std::vector<std::string> required{};
    for (auto&& p : testParticipants)
    {
        required.push_back(p.name);
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
    JoinParticipantThreads(_participantThreads_Sync_Coordinated);

    AbortSystemController();
    StopRegistry();
}

// -----------------
// Mode combinations

TEST_F(ITest_Internals_ParticipantModes, test_AsyncAutonomousReq_with_SyncCoordinated)
{
    RunRegistry();

    // Participants
    std::list<TestParticipant> testParticipants;
    testParticipants.push_back({"SyncAutonomous1", TimeMode::Async, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous2", TimeMode::Async, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous3", TimeMode::Async, OperationMode::Autonomous});
    testParticipants.push_back({"SyncAutonomous4", TimeMode::Async, OperationMode::Autonomous});
    testParticipants.push_back({"SyncCoordinated1", TimeMode::Sync, OperationMode::Coordinated});
    testParticipants.push_back({"SyncCoordinated2", TimeMode::Sync, OperationMode::Coordinated});
    testParticipants.push_back({"SyncCoordinated3", TimeMode::Sync, OperationMode::Coordinated});
    testParticipants.push_back({"SyncCoordinated4", TimeMode::Sync, OperationMode::Coordinated});
    expectedReceptions = testParticipants.size();

    // All are required here
    std::vector<std::string> required{};
    for (auto&& p : testParticipants)
    {
        required.push_back(p.name);
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
    JoinParticipantThreads(_participantThreads_Async_Autonomous);
    JoinParticipantThreads(_participantThreads_Sync_Coordinated);

    AbortSystemController();
    StopRegistry();
}


TEST_F(ITest_Internals_ParticipantModes, test_AsyncInvalid_with_SyncAutonomous)
{
    RunRegistry();

    // Participants
    std::list<TestParticipant> testParticipants;
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
    JoinParticipantThreads(_participantThreads_Async_Invalid);
    JoinParticipantThreads(participantThreads_Sync_Autonomous);

    StopRegistry();
}

// ----------------
// All combinations
// ----------------

TEST_F(ITest_Internals_ParticipantModes, test_Combinations)
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
                    if (verbose)
                    {
                        std::cout << "P1:" << timeModeNames[p1_timeMode] << " + "
                                  << operationModesNames[p1_operationMode] << " P2:" << timeModeNames[p2_timeMode]
                                  << " + " << operationModesNames[p2_operationMode];
                    }

                    if ((p1_timeMode == TimeMode::Sync && p1_operationMode == OperationMode::Invalid)
                        || (p2_timeMode == TimeMode::Sync && p2_operationMode == OperationMode::Invalid))
                    {
                        std::cout << " -> Invalid combination (Sync+Invalid), skip" << std::endl;
                        continue;
                    }

                    if ((p1_timeMode == TimeMode::Sync && p1_operationMode == OperationMode::Autonomous
                         && p2_timeMode == TimeMode::Sync && p2_operationMode == OperationMode::Coordinated)
                        || (p2_timeMode == TimeMode::Sync && p2_operationMode == OperationMode::Autonomous
                            && p1_timeMode == TimeMode::Sync && p1_operationMode == OperationMode::Coordinated))
                    {
                        std::cout << " -> Invalid combination (Sync+Autonomous with Sync+Coordinated), skip"
                                  << std::endl;
                        continue;
                    }

                    // Participants
                    std::list<TestParticipant> testParticipants;
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
                    if (!_participantThreads_Sync_Invalid.empty())
                    {
                        JoinParticipantThreads(_participantThreads_Sync_Invalid);
                    }
                    if (!participantThreads_Sync_Autonomous.empty())
                    {
                        JoinParticipantThreads(participantThreads_Sync_Autonomous);
                    }
                    if (!_participantThreads_Sync_Coordinated.empty())
                    {
                        JoinParticipantThreads(_participantThreads_Sync_Coordinated);
                    }
                    if (!_participantThreads_Async_Invalid.empty())
                    {
                        JoinParticipantThreads(_participantThreads_Async_Invalid);
                    }
                    if (!_participantThreads_Async_Autonomous.empty())
                    {
                        JoinParticipantThreads(_participantThreads_Async_Autonomous);
                    }
                    if (!_participantThreads_Async_Coordinated.empty())
                    {
                        JoinParticipantThreads(_participantThreads_Async_Coordinated);
                    }
                    if (!required.empty())
                    {
                        AbortSystemController();
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
