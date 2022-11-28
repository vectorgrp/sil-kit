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

class TestDashboardCachingSilKitEventHandler : public Test
{
public:
    void SetUp() override
    {
        _mockEventHandler = std::make_shared<StrictMock<MockSilKitEventHandler>>();

        EXPECT_CALL(_dummyLogger, GetLogLevel).WillRepeatedly(Return(Services::Logging::Level::Info));
    }

    std::shared_ptr<CachingSilKitEventHandler> CreateService(const std::string& participantName,
                                                             CachingSilKitEventHandlerState state = Caching)
    {
        auto service = std::make_shared<CachingSilKitEventHandler>(&_dummyLogger, participantName, _mockEventHandler);
        service->_state = state;
        return service;
    }

    template <typename T>
    std::shared_ptr<CachingSilKitEventHandler> CreateService(const std::string& participantName,
                                                             CachingSilKitEventHandlerState state, const T& cacheDatum)
    {
        auto service = CreateService(participantName, state);
        service->_dataCache.Insert(cacheDatum);
        return service;
    }

    void CheckState(std::shared_ptr<CachingSilKitEventHandler> service, CachingSilKitEventHandlerState expectedState)
    {
        ASSERT_EQ(service->_state, expectedState) << "Wrong state!";
    }

    template <typename T>
    void CheckCache(std::shared_ptr<CachingSilKitEventHandler> service, std::vector<T> expected)
    {
        auto actual = service->_dataCache.GetAndClear<T>();
        ASSERT_EQ(actual.size(), expected.size()) << "Vectors have different size!";
        for (int i = 0; i < expected.size(); ++i)
        {
            ASSERT_EQ(actual[i], expected[i]) << "Vectors differ at index " << i;
        }
    }

    Core::Tests::MockLogger _dummyLogger;
    std::shared_ptr<StrictMock<MockSilKitEventHandler>> _mockEventHandler;
};

TEST_F(TestDashboardCachingSilKitEventHandler, Inital_StateCaching)
{
    // Arrange

    // Act
    const auto service = CreateService("SilKitDashboard");

    // Assert
    CheckState(service, CachingSilKitEventHandlerState::Caching);
}

TEST_F(TestDashboardCachingSilKitEventHandler, OnStart_Failure_SetDisabledState)
{
    // Arrange
    std::promise<bool> simulationCreated;
    EXPECT_CALL(*_mockEventHandler, OnStart).WillOnce(Return(simulationCreated.get_future()));
    EXPECT_CALL(_dummyLogger, Warn("Dashboard: simulation creation failed, disabling caching")).WillOnce(Return());
    const auto service = CreateService("SilKitDashboard");

    // Act
    auto res = service->OnStart("silkit://localhost:8500", 0);
    simulationCreated.set_value(false);

    // Assert
    ASSERT_FALSE(res.get());
    CheckState(service, CachingSilKitEventHandlerState::Disabled);
}

TEST_F(TestDashboardCachingSilKitEventHandler, OnStart_StateCachingAndSuccess_SetSendingStateAndNotifyCachedEvents)
{
    // Arrange
    std::promise<bool> simulationCreated;
    EXPECT_CALL(*_mockEventHandler, OnStart).WillOnce(Return(simulationCreated.get_future()));
    EXPECT_CALL(_dummyLogger, Info("Dashboard: notifying cached events"))
        .WillOnce(Return());
    Services::Orchestration::ParticipantConnectionInformation expectedInfo;
    Services::Orchestration::ParticipantConnectionInformation actualInfo;
    EXPECT_CALL(*_mockEventHandler, OnParticipantConnected).WillOnce(DoAll(WithArgs<0>([&](auto info) {
        actualInfo = info;
    })));
    const auto service = CreateService("SilKitDashboard", Caching, expectedInfo);

    // Act
    auto res = service->OnStart("silkit://localhost:8500", 0);
    simulationCreated.set_value(true);

    // Assert
    ASSERT_TRUE(res.get());
    CheckState(service, CachingSilKitEventHandlerState::Sending);
    std::vector<Services::Orchestration::ParticipantConnectionInformation> expected{};
    CheckCache(service, expected);
    ASSERT_EQ(actualInfo, expectedInfo) << "Wrong ParticipantConnectionInformation!";
}


TEST_F(TestDashboardCachingSilKitEventHandler, OnShutdown_StateCaching_SetDisabledState)
{
    // Arrange
    EXPECT_CALL(_dummyLogger, Warn("Dashboard: not sending, skipping setting an end")).WillOnce(Return());
    const auto service = CreateService("SilKitDashboard");

    // Act
    service->OnShutdown(0);

    // Assert
    CheckState(service, CachingSilKitEventHandlerState::Disabled);
}

TEST_F(TestDashboardCachingSilKitEventHandler, OnShutdown_StateSending_SetDisabledStateAndSend)
{
    // Arrange
    uint64_t actualStopTime;
    EXPECT_CALL(*_mockEventHandler, OnShutdown).WillOnce(DoAll(WithArgs<0>([&](auto stopTime) {
        actualStopTime = stopTime;
    })));
    const auto service = CreateService("SilKitDashboard", Sending);

    // Act
    const uint64_t expectedStopTime = 123456;
    service->OnShutdown(expectedStopTime);

    // Assert
    CheckState(service, CachingSilKitEventHandlerState::Disabled);
    ASSERT_EQ(actualStopTime, expectedStopTime) << "Wrong time!";
}

TEST_F(TestDashboardCachingSilKitEventHandler, OnShutdown_StateDisabled_SetDisabledState)
{
    // Arrange
    EXPECT_CALL(_dummyLogger, Warn("Dashboard: not sending, skipping setting an end"))
        .WillOnce(Return());
    const auto service = CreateService("SilKitDashboard", Disabled);

    // Act
    const uint64_t stopTime = 123456;
    service->OnShutdown(stopTime);

    // Assert
    CheckState(service, CachingSilKitEventHandlerState::Disabled);
}

TEST_F(TestDashboardCachingSilKitEventHandler, OnStartOnShutdown_NoResponse_SetDisabledState)
{
    // Arrange
    std::promise<bool> simulationCreated;
    EXPECT_CALL(*_mockEventHandler, OnStart).WillOnce(Return(simulationCreated.get_future()));
    EXPECT_CALL(_dummyLogger, Warn("Dashboard: not sending, skipping setting an end"))
        .WillOnce(Return());
    EXPECT_CALL(_dummyLogger, Warn("Dashboard: already disabled")).WillOnce(Return());
    const auto service = CreateService("SilKitDashboard");

    // Act
    auto res = service->OnStart("silkit://localhost:8500", 0);
    service->OnShutdown(0);

    // Assert
    ASSERT_FALSE(res.get());
    CheckState(service, CachingSilKitEventHandlerState::Disabled);
}

TEST_F(TestDashboardCachingSilKitEventHandler, OnParticipantConnected_IgnoreOwnEvents)
{
    // Arrange
    const auto participantName = "SilKitDashboard";
    const auto service = CreateService(participantName);

    // Act
    Services::Orchestration::ParticipantConnectionInformation info;
    info.participantName = participantName;
    service->OnParticipantConnected(info);

    // Assert
    std::vector<Services::Orchestration::ParticipantConnectionInformation> expected{};
    CheckCache(service, expected);
}

TEST_F(TestDashboardCachingSilKitEventHandler, OnParticipantConnected_StateCaching_InsertInCache)
{
    // Arrange
    const auto service = CreateService("SilKitDashboard");

    // Act
    Services::Orchestration::ParticipantConnectionInformation expectedInfo;
    service->OnParticipantConnected(expectedInfo);

    // Assert
    std::vector<Services::Orchestration::ParticipantConnectionInformation> expected{expectedInfo};
    CheckCache(service, expected);
}

TEST_F(TestDashboardCachingSilKitEventHandler, OnParticipantConnected_StateSending_Send)
{
    // Arrange
    Services::Orchestration::ParticipantConnectionInformation actualInfo;
    EXPECT_CALL(*_mockEventHandler, OnParticipantConnected).WillOnce(DoAll(WithArgs<0>([&](auto info) {
        actualInfo = info;
    })));
    const auto service = CreateService("SilKitDashboard", Sending);

    // Act
    Services::Orchestration::ParticipantConnectionInformation expectedInfo;
    service->OnParticipantConnected(expectedInfo);

    // Assert
    ASSERT_EQ(actualInfo, expectedInfo) << "Wrong ParticipantConnectionInformation!";
    std::vector<Services::Orchestration::ParticipantConnectionInformation> expected;
    CheckCache(service, expected);
}

TEST_F(TestDashboardCachingSilKitEventHandler, OnParticipantConnected_StateDisabled_Ignore)
{
    // Arrange
    const auto service = CreateService("SilKitDashboard", Disabled);

    // Act
    Services::Orchestration::ParticipantConnectionInformation info;
    service->OnParticipantConnected(info);

    // Assert
    std::vector<Services::Orchestration::ParticipantConnectionInformation> expected;
    CheckCache(service, expected);
}

} // namespace Dashboard
} // namespace SilKit
