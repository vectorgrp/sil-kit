// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <thread>
#include <future>
#include <string>
#include <sstream>
#include <numeric>

#include "CreateComAdapter.hpp"
#include "VAsioRegistry.hpp"

#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/sim/all.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

namespace {

using namespace std::chrono;
using namespace ib::cfg;
using namespace ib::mw;
using namespace ib::mw::sync;
using namespace ib::sim::generic;

const std::string testMessage{"TestMessage"};

std::ostream& operator<<(std::ostream& out, const nanoseconds& timestamp)
{
    out << timestamp.count();
    return out;
}

std::istream& operator>>(std::istream& in, nanoseconds& timestamp)
{
    nanoseconds::rep timeRep;
    in >> timeRep;
    timestamp = nanoseconds{timeRep};
    return in;
}

class Publisher
{
public:
    Publisher(const Config& ibConfig, const uint32_t domainId, const uint32_t publisherIndex, const uint32_t testSize)
        : _testSize{testSize}
    {
        _comAdapter = CreateComAdapterImpl(ibConfig, "Publisher" + std::to_string(publisherIndex));
        _comAdapter->joinIbDomain(domainId);

        const auto topicName = "GenericData" + std::to_string(publisherIndex);
        auto&& participantController = _comAdapter->GetParticipantController();
        auto* publisher = _comAdapter->CreateGenericPublisher(topicName);

        participantController->SetSimulationTask(
            [this, publisher](const nanoseconds now, nanoseconds /*duration*/) {

            if (_messageIndex < _testSize)
            {
                PublishMessage(publisher, now, _messageIndex++);
            }
        });
    }

    void RunAsync()
    {
        auto&& participantController = _comAdapter->GetParticipantController();
        _simulationFuture = participantController->RunAsync();
    }

    auto WaitForShutdown() -> ParticipantState
    {
        return _simulationFuture.get();
    }

    uint32_t NumMessagesSent() const
    {
        return _messageIndex;
    }

private:
    void PublishMessage(IGenericPublisher* publisher, const nanoseconds now, const uint32_t index) const
    {
        // Send testMessage with current tick (now) and current index
        std::stringstream stream;
        stream << testMessage << ' ' << now << ' ' << index;
        auto message = stream.str();
        std::vector<uint8_t> testData{message.begin(), message.end()};

        publisher->Publish(std::move(testData));
    }

private:
    std::unique_ptr<IComAdapterInternal> _comAdapter{nullptr};

    uint32_t _messageIndex{0u};
    uint32_t _testSize{0u};
    std::future<ParticipantState> _simulationFuture;
};

class Subscriber
{
public:
    Subscriber(Config ibConfig, const uint32_t domainId, const uint32_t& publisherCount, const uint32_t testSize)
        : _ibConfig{std::move(ibConfig)}
        , _publisherCount{publisherCount}
        , _messageIndexes(publisherCount, 0u)
        , _testSize{testSize}
    {
        _comAdapter = CreateComAdapterImpl(_ibConfig, "Subscriber");
        _comAdapter->joinIbDomain(domainId);

        _systemController = _comAdapter->GetSystemController();
        _monitor = _comAdapter->GetSystemMonitor();

        _monitor->RegisterSystemStateHandler(
            [this](SystemState newState) {
            this->OnSystemStateChanged(newState);
        });

        auto&& participantController = _comAdapter->GetParticipantController();

        for (auto publisherIndex = 0u; publisherIndex < _publisherCount; publisherIndex++)
        {
            auto* subscriber = _comAdapter->CreateGenericSubscriber("GenericData" + std::to_string(publisherIndex));
            subscriber->SetReceiveMessageHandler(
                [this, publisherIndex](IGenericSubscriber* subscriber, const std::vector<uint8_t>& data) {
                ReceiveMessage(subscriber, data, publisherIndex);
            });
        }

        participantController->SetSimulationTask(
            [this](const nanoseconds now, nanoseconds /*duration*/) {

            _currentTick = now;

        });
    }

    std::future<ParticipantState> RunAsync() const
    {
        auto&& participantController = _comAdapter->GetParticipantController();
        return participantController->RunAsync();
    }

    uint32_t NumMessagesReceived(const uint32_t publisherIndex)
    {
        return _messageIndexes[publisherIndex];
    }

private:
    void OnSystemStateChanged(SystemState newState)
    {
        if (newState == SystemState::Idle)
        {
            InitializeAllParticipants();
        }
        else if (newState == SystemState::Initialized)
        {
            _systemController->Run();
        }
        else if (newState == SystemState::Stopped)
        {
            _systemController->Shutdown();
        }
    }

    void InitializeAllParticipants()
    {
        for (auto&& participant : _ibConfig.simulationSetup.participants)
        {
            if (!participant.participantController)
                continue;
            _systemController->Initialize(participant.name);
        }
    }

    void ReceiveMessage(IGenericSubscriber* /*subscriber*/, const std::vector<uint8_t>& data, const uint32_t publisherIndex)
    {
        auto& messageIndex = _messageIndexes[publisherIndex];
        const std::string message{data.begin(), data.end()};

        std::string receivedMessage;
        nanoseconds sentTick = {};
        uint32_t receivedIndex;
        std::stringstream stream{message};

        try
        {
            stream >> receivedMessage >> sentTick >> receivedIndex;
        }
        catch (std::runtime_error& /*error*/)
        {
            std::cout << "ERROR: Received message does not match the expected format" << std::endl;
            _systemController->Stop();
            return;
        }

        // The constant testMessage should have been sent and received correctly
        EXPECT_EQ(testMessage, receivedMessage);

        // Exactly one message per publisher should be sent and received in the same simulation tick.

        // currentTick is incremented by one millisecond with every simulation tick.
        // So sentTick should be equal to currentTick or one millisecond (=tick) further,
        // if currentTick has not yet been updated in the current tick.
        ASSERT_TRUE(_currentTick == sentTick || _currentTick + _ibConfig.simulationSetup.timeSync.tickPeriod == sentTick);

        // This expectation tests the order of the messages per publisher.
        // For each publisher each send message should come in order. The order is tested 
        // by incrementing the messageIndex for each message in the Publisher and Subscriber.
        ASSERT_EQ(messageIndex, receivedIndex);

        messageIndex++;

        const auto sumOfIndexes = std::accumulate(_messageIndexes.begin(), _messageIndexes.end(), 0u);
        if (sumOfIndexes == _testSize * _publisherCount)
        {
            _systemController->Stop();
        }
    }

private:
    std::unique_ptr<IComAdapterInternal> _comAdapter{nullptr};
    Config _ibConfig;
    ISystemController* _systemController{nullptr};
    ISystemMonitor* _monitor{nullptr};

    uint32_t _publisherCount{0u};
    std::vector<uint32_t> _messageIndexes;
    uint32_t _testSize{0u};

    nanoseconds _currentTick{0ns};
};

class DeterministicSimVAsioITest : public testing::Test
{
protected:
    DeterministicSimVAsioITest()
    {
        domainId = static_cast<uint32_t>(GetTestPid());
    }

    void buildConfig(const uint32_t publisherCount)
    {
        ConfigBuilder builder{"TestConfig"};

        auto&& subscriber = builder.SimulationSetup().AddParticipant("Subscriber").AsSyncMaster();
        subscriber.AddParticipantController().WithSyncType(SyncType::DistributedTimeQuantum);

        for (auto i = 0u; i < publisherCount; i++)
        {
            subscriber.AddGenericSubscriber("GenericData" + std::to_string(i)).WithLink("GenericData" + std::to_string(i));

            auto&& participantB = builder.SimulationSetup().AddParticipant("Publisher" + std::to_string(i));
            participantB.AddParticipantController().WithSyncType(SyncType::DistributedTimeQuantum);
            participantB.AddGenericPublisher("GenericData" + std::to_string(i)).WithLink("GenericData" + std::to_string(i));
        }

        builder.WithActiveMiddleware(Middleware::VAsio).SimulationSetup()
            .ConfigureTimeSync()
            .WithTickPeriod(std::chrono::milliseconds(1));

        ibConfig = builder.Build();
    }

protected:
    uint32_t domainId;
    Config ibConfig;
};

TEST_F(DeterministicSimVAsioITest, deterministic_simulation_vasio)
{
    const uint32_t publisherCount = 3;
    const uint32_t testSize = 5000;

    buildConfig(publisherCount);

    VAsioRegistry registry{ibConfig};
    registry.ProvideDomain(domainId);

    // The subscriber takes part of the system controller and initiates simulation state changes
    Subscriber subscriber(ibConfig, domainId, publisherCount, testSize);
    auto subscriberFuture = subscriber.RunAsync();

    std::vector<Publisher> publishers;
    publishers.reserve(publisherCount);
    for (auto i = 0u; i < publisherCount; i++)
    {
        publishers.emplace_back(ibConfig, domainId, i, testSize);
        publishers[i].RunAsync();
    }

    // The important expectations of this test are in the ReceiveMessage method of the Subscriber:
    //
    //   1. The sent message (from Publisher1, 2, 3 ...) has to be received in the same tick.
    //   2. The received messages have to come in order (Index 1 before 2 before 3, ...):
    //(e.g.): PubA - Test Message: 1
    //        PubB - Test Message: 1
    //        PubB - Test Message: 2
    //        PubA - Test Message: 2
    //        PubB - Test Message: 3
    //        PubA - Test Message: 3
    //        ...

    auto finalState = subscriberFuture.get();
    EXPECT_EQ(ParticipantState::Shutdown, finalState);

    for (auto publisherIndex = 0u; publisherIndex < publisherCount; publisherIndex++)
    {
        auto& publisher = publishers[publisherIndex];

        EXPECT_EQ(ParticipantState::Shutdown, publisher.WaitForShutdown());
        EXPECT_EQ(testSize, publisher.NumMessagesSent());
        EXPECT_EQ(testSize, subscriber.NumMessagesReceived(publisherIndex));
    }
}

} // anonymous namespace
