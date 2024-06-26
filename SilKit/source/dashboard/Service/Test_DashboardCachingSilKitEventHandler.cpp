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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "MockParticipant.hpp"

#include "CachingSilKitEventHandler.hpp"

#include "Mocks/MockSilKitEventQueue.hpp"
#include "Mocks/MockSilKitEventHandler.hpp"

using namespace testing;

namespace SilKit {

namespace Services {
namespace Orchestration {
bool operator==(const ParticipantConnectionInformation& lhs, const ParticipantConnectionInformation& rhs)
{
    return lhs.participantName == rhs.participantName;
}
} // namespace Orchestration
} // namespace Services

namespace Dashboard {

class Test_DashboardCachingSilKitEventHandler : public Test
{
public:
    void SetUp() override
    {
        _mockEventHandler = std::make_shared<StrictMock<MockSilKitEventHandler>>();
        _mockEventQueue = std::make_shared<StrictMock<MockSilKitEventQueue>>();
        EXPECT_CALL(_dummyLogger, GetLogLevel).WillRepeatedly(Return(Services::Logging::Level::Warn));
    }

    std::shared_ptr<CachingSilKitEventHandler> CreateService()
    {
        return std::make_shared<CachingSilKitEventHandler>(_connectUri, &_dummyLogger, _mockEventHandler,
                                                           _mockEventQueue);
    }

    void CheckConnectUri(const std::string& actual)
    {
        ASSERT_EQ(actual, _connectUri) << "Wrong connectUri!";
    }

    void CheckTime(uint64_t actual)
    {
        ASSERT_TRUE(actual > 0) << "Wrong time!";
    }

    void CheckSimulationId(uint64_t actual)
    {
        ASSERT_EQ(actual, _simulationId) << "Wrong simulationId!";
    }

    Core::Tests::MockLogger _dummyLogger;
    std::shared_ptr<StrictMock<MockSilKitEventHandler>> _mockEventHandler;
    std::shared_ptr<StrictMock<MockSilKitEventQueue>> _mockEventQueue;
    const std::string _connectUri{"silkit://localhost:8500"};
    const uint64_t _simulationId{123};
    const uint64_t _invalidSimulationId{0};
};

TEST_F(Test_DashboardCachingSilKitEventHandler, NoEvents)
{
    // Arrange
    EXPECT_CALL(*_mockEventQueue, DequeueAllInto)
        .WillOnce(DoAll(WithArgs<0>([&](auto& evts) { evts.clear(); }), Return(false)));
    EXPECT_CALL(*_mockEventQueue, Stop);

    // Act
    {
        const auto service = CreateService();
    }

    // Assert
}

TEST_F(Test_DashboardCachingSilKitEventHandler, OnParticipantConnected_CreateSimulationFailure)
{
    // Arrange
    bool simulationStartFound = false;
    bool participantConnectionInformationFound = false;
    EXPECT_CALL(*_mockEventQueue, Enqueue).WillRepeatedly(WithArgs<0>([&](const auto& evt) {
        switch (evt.Type())
        {
        case SilKitEventType::OnSimulationStart:
            simulationStartFound = true;
            break;
        case SilKitEventType::OnParticipantConnected:
            participantConnectionInformationFound = true;
            break;
        default: /* do nothing */
            break;
        }
    }));

    Services::Orchestration::ParticipantConnectionInformation participantConnectionInformation;
    EXPECT_CALL(*_mockEventQueue, DequeueAllInto)
        .WillOnce(DoAll(WithArgs<0>([&](auto& evts) {
        SimulationStart simulationStart{"silkit://localhost:8500", 123456};
        std::vector<SilKitEvent> events;
        events.emplace_back("", simulationStart);
        events.emplace_back("", participantConnectionInformation);
        evts.swap(events);
    }),
                        Return(true)))
        .WillOnce(DoAll(WithArgs<0>([&](auto& evts) { evts.clear(); }), Return(false)));
    std::string actualConnectUri;
    uint64_t actualTime = 0;
    EXPECT_CALL(*_mockEventHandler, OnSimulationStart)
        .WillOnce(DoAll(WithArgs<0, 1>([&](auto connectUri, auto time) {
        actualConnectUri = connectUri;
        actualTime = time;
    }),
                        Return(_invalidSimulationId)));
    EXPECT_CALL(*_mockEventQueue, Stop);

    // Act
    {
        const auto service = CreateService();
        service->OnParticipantConnected(participantConnectionInformation);
    }

    // Assert
    ASSERT_TRUE(simulationStartFound) << "No simulation start event produced!";
    ASSERT_TRUE(participantConnectionInformationFound) << "No participant connection event produced!";
    CheckConnectUri(actualConnectUri);
    CheckTime(actualTime);
}

TEST_F(Test_DashboardCachingSilKitEventHandler, OnParticipantConnected_CreateSimulationSuccess)
{
    // Arrange
    bool simulationStartFound = false;
    bool participantConnectionInformationFound = false;
    EXPECT_CALL(*_mockEventQueue, Enqueue).WillRepeatedly(WithArgs<0>([&](const auto& evt) {
        switch (evt.Type())
        {
        case SilKitEventType::OnSimulationStart:
            simulationStartFound = true;
            break;
        case SilKitEventType::OnParticipantConnected:
            participantConnectionInformationFound = true;
            break;
        default: /* do nothing */
            break;
        }
    }));
    Services::Orchestration::ParticipantConnectionInformation participantConnectionInformation;
    EXPECT_CALL(*_mockEventQueue, DequeueAllInto)
        .WillOnce(DoAll(WithArgs<0>([&](auto& evts) {
        SimulationStart simulationStart{"silkit://localhost:8500", 123456};
        std::vector<SilKitEvent> events;
        events.emplace_back("", simulationStart);
        events.emplace_back("", participantConnectionInformation);
        evts.swap(events);
    }),
                        Return(true)))
        .WillOnce(DoAll(WithArgs<0>([&](auto& evts) { evts.clear(); }), Return(false)));
    std::string actualConnectUri;
    uint64_t actualTime = 0;
    EXPECT_CALL(*_mockEventHandler, OnSimulationStart)
        .WillOnce(DoAll(WithArgs<0, 1>([&](auto connectUri, auto time) {
        actualConnectUri = connectUri;
        actualTime = time;
    }),
                        Return(_simulationId)));
    uint64_t actualSimulationId = 0;
    Services::Orchestration::ParticipantConnectionInformation actualInfo;
    EXPECT_CALL(*_mockEventHandler, OnParticipantConnected).WillOnce(WithArgs<0, 1>([&](auto simulationId, auto info) {
        actualSimulationId = simulationId;
        actualInfo = info;
    }));
    EXPECT_CALL(*_mockEventQueue, Stop);

    // Act
    {
        const auto service = CreateService();
        service->OnParticipantConnected(participantConnectionInformation);
    }

    // Assert
    ASSERT_TRUE(simulationStartFound) << "No simulation start event produced!";
    ASSERT_TRUE(participantConnectionInformationFound) << "No participant connection event produced!";
    CheckConnectUri(actualConnectUri);
    CheckTime(actualTime);
    CheckSimulationId(actualSimulationId);
    ASSERT_EQ(actualInfo, participantConnectionInformation) << "Wrong ParticipantConnectionInformation!";
}

TEST_F(Test_DashboardCachingSilKitEventHandler, OnLastParticipantDisconnected_SimulationNotRunning)
{
    // Arrange
    EXPECT_CALL(*_mockEventQueue, DequeueAllInto)
        .WillOnce(DoAll(WithArgs<0>([&](auto& evts) { evts.clear(); }), Return(false)));
    EXPECT_CALL(*_mockEventQueue, Stop);

    // Act
    {
        const auto service = CreateService();
        service->OnLastParticipantDisconnected();
    }

    // Assert
}

} // namespace Dashboard
} // namespace SilKit
