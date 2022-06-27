// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <thread>
#include <future>
#include <string>
#include <sstream>
#include <numeric>

#include "CreateParticipant.hpp"
#include "VAsioRegistry.hpp"

#include "ib/mw/sync/all.hpp"
#include "ib/sim/all.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"
#include "MockParticipantConfiguration.hpp"

namespace {

using namespace std::chrono;
using namespace ib::cfg;
using namespace ib::mw;
using namespace ib::mw::sync;
using namespace ib::sim::data;

const std::string testMessage{"TestMessage"};
std::vector<std::string> syncParticipantNames;
auto period = std::chrono::milliseconds(1);

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
    Publisher(const uint32_t domainId, const uint32_t publisherIndex, const uint32_t testSize)
        : _testSize{testSize}
    {
        _participantName = "Publisher" + std::to_string(publisherIndex);
        _participant =
            ib::mw::CreateParticipantImpl(ib::cfg::MockParticipantConfiguration(), _participantName);

        _participant->joinIbDomain(domainId);

        const auto topicName = "Topic" + std::to_string(publisherIndex);
        auto* lifecycleService = _participant->GetLifecycleService();
        auto* timeSyncService = lifecycleService->GetTimeSyncService();
        auto* publisher = _participant->CreateDataPublisher("PubCtrl1", topicName, {}, {}, 0);

        timeSyncService->SetPeriod(period);
        timeSyncService->SetSimulationTask(
            [this, publisher](const nanoseconds now, nanoseconds /*duration*/) {

            if (_messageIndex < _testSize)
            {
                PublishMessage(publisher, now, _messageIndex++);
            }
        });
    }

    void RunAsync()
    {
        auto* lifecycleService = _participant->GetLifecycleService();
        _simulationFuture = lifecycleService->StartLifecycleWithSyncTime(lifecycleService->GetTimeSyncService(), {true, true});
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
    void PublishMessage(IDataPublisher* publisher, const nanoseconds now, const uint32_t index) const
    {
        // Send testMessage with current tick (now) and current index
        std::stringstream stream;
        stream << testMessage << ' ' << now << ' ' << index;
        auto message = stream.str();
        std::vector<uint8_t> testData{message.begin(), message.end()};

        publisher->Publish(std::move(testData));
    }

private:
    std::unique_ptr<IParticipantInternal> _participant{nullptr};

    uint32_t _messageIndex{0u};
    uint32_t _testSize{0u};
    std::future<ParticipantState> _simulationFuture;
    std::string  _participantName;
};

class Subscriber
{
public:
    Subscriber(const std::string& participantName, const uint32_t domainId, const uint32_t& publisherCount, const uint32_t testSize)
        : _publisherCount{publisherCount}
        , _messageIndexes(publisherCount, 0u)
        , _testSize{testSize}
        , _participantName{participantName}
    {
        _participant = ib::mw::CreateParticipantImpl(
            ib::cfg::MockParticipantConfiguration(), participantName);
        _participant->joinIbDomain(domainId);

        _systemController = _participant->GetSystemController();
        _systemController->SetWorkflowConfiguration({syncParticipantNames});

        _monitor = _participant->GetSystemMonitor();
        _monitor->AddSystemStateHandler([this](SystemState newState) {
            this->OnSystemStateChanged(newState);
        });

        auto* lifecycleService = _participant->GetLifecycleService();
        auto* timeSyncService = lifecycleService->GetTimeSyncService();

        for (auto publisherIndex = 0u; publisherIndex < _publisherCount; publisherIndex++)
        {
            _participant->CreateDataSubscriber(
                "SubCtrl1", "Topic" + std::to_string(publisherIndex), {}, {},
                [this, publisherIndex](IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent) {
                    ReceiveMessage(subscriber, dataMessageEvent, publisherIndex);
                });
        }
        timeSyncService->SetPeriod(period);
        timeSyncService->SetSimulationTask(
            [this](const nanoseconds now, nanoseconds /*duration*/) {

            _currentTick = now;

        });
    }

    std::future<ParticipantState> RunAsync() const
    {
        auto* lifecycleService = _participant->GetLifecycleService();
        return lifecycleService->StartLifecycleWithSyncTime(lifecycleService->GetTimeSyncService(), {true, true});
    }

    uint32_t NumMessagesReceived(const uint32_t publisherIndex)
    {
        return _messageIndexes[publisherIndex];
    }

private:
    void OnSystemStateChanged(SystemState newState)
    {
        if (newState == SystemState::ReadyToRun)
        {
            _systemController->Run();
        }
        else if (newState == SystemState::Stopped)
        {
            for(auto&& name: syncParticipantNames)
            {
                _systemController->Shutdown(name);
            }
        }
    }

    void ReceiveMessage(IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent, const uint32_t publisherIndex)
    {
        auto& messageIndex = _messageIndexes[publisherIndex];
        const std::string message{ dataMessageEvent.data.begin(), dataMessageEvent.data.end()};

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
            for(auto&& name: syncParticipantNames)
            {
                _systemController->Shutdown(name);
            }
            _systemController->Stop();
            return;
        }

        // The constant testMessage should have been sent and received correctly
        EXPECT_EQ(testMessage, receivedMessage);

        // Exactly one message per publisher should be sent and received in the same simulation tick.

        // currentTick is incremented by one millisecond with every simulation tick.
        // So sentTick should be equal to currentTick or one millisecond (=tick) further,
        // if currentTick has not yet been updated in the current tick.
        ASSERT_TRUE(_currentTick == sentTick || _currentTick + period == sentTick);

        // This expectation tests the order of the messages per publisher.
        // For each publisher each send message should come in order. The order is tested 
        // by incrementing the messageIndex for each message in the Publisher and Subscriber.
        ASSERT_EQ(messageIndex, receivedIndex);

        messageIndex++;

        const auto sumOfIndexes = std::accumulate(_messageIndexes.begin(), _messageIndexes.end(), 0u);
        if (sumOfIndexes == _testSize * _publisherCount)
        {
            _systemController->Stop();
            for(auto&& name: syncParticipantNames)
            {
                _systemController->Shutdown(name);
            }
        }
    }

private:
    std::unique_ptr<IParticipantInternal> _participant{nullptr};
    ISystemController* _systemController{nullptr};
    ISystemMonitor* _monitor{nullptr};

    uint32_t _publisherCount{0u};
    std::vector<uint32_t> _messageIndexes;
    uint32_t _testSize{0u};

    nanoseconds _currentTick{0ns};
    std::string _participantName;
};

class DeterministicSimVAsioITest : public testing::Test
{
protected:
    DeterministicSimVAsioITest()
    {
        domainId = static_cast<uint32_t>(GetTestPid());
    }

protected:
    uint32_t domainId;
};

TEST_F(DeterministicSimVAsioITest, deterministic_simulation_vasio)
{
    const uint32_t publisherCount = 3;
    const uint32_t testSize = 5000;

    std::string subscriberName = "Subscriber";
    syncParticipantNames.push_back(subscriberName);
    for (auto i = 0u; i < publisherCount; i++)
    {
        syncParticipantNames.push_back("Publisher" + std::to_string(i));
    }

    VAsioRegistry registry{ ib::cfg::MockParticipantConfiguration() };
    registry.ProvideDomain(domainId);

    // The subscriber assumes the role of the system controller and initiates simulation state changes
    Subscriber subscriber(subscriberName, domainId, publisherCount, testSize);
    auto subscriberFuture = subscriber.RunAsync();

    std::vector<Publisher> publishers;
    publishers.reserve(publisherCount);
    for (auto i = 0u; i < publisherCount; i++)
    {
        publishers.emplace_back(domainId, i, testSize);
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
