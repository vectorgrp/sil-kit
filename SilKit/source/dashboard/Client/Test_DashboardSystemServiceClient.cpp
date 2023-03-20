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

#include "DashboardSystemServiceClient.hpp"
#include "Mocks/MockBodyDecoder.hpp"
#include "Mocks/MockDashboardSystemApiClient.hpp"
#include "Mocks/MockInputStream.hpp"
#include "Mocks/MockObjectMapper.hpp"

using namespace oatpp::web;
using namespace oatpp::data::mapping;
using namespace protocol::http;
using namespace testing;

namespace SilKit {
namespace Dashboard {

class MockDecodeAsync : public oatpp::async::Coroutine<MockDecodeAsync>
{
public:
    MockDecodeAsync() {}

public:
    Action act() override { return std::forward<Action>(finish()); }
};

class MockExecuteRequestAsync
    : public oatpp::async::CoroutineWithResult<MockExecuteRequestAsync, const std::shared_ptr<incoming::Response> &>
{
public:
    MockExecuteRequestAsync(Status status, std::shared_ptr<MockInputStream> nullInput,
                            std::shared_ptr<StrictMock<MockBodyDecoder>> mockBodyDecoder)
        : _status(status)
        , _nullInput(nullInput)
        , _mockBodyDecoder(mockBodyDecoder)
    {
    }

public:
    Action act() override
    {
        oatpp::data::share::LazyStringMultimap<oatpp::data::share::StringKeyLabelCI> headers;
        return _return(
            incoming::Response::createShared(_status.code, _status.description, headers, _nullInput, _mockBodyDecoder));
    }

private:
    Status _status;
    std::shared_ptr<MockInputStream> _nullInput;
    std::shared_ptr<StrictMock<MockBodyDecoder>> _mockBodyDecoder;
};

class TestDashboardSystemServiceClient : public Test
{
public:
    void SetUp() override
    {
        _mockObjectMapper = std::make_shared<MockObjectMapper>(_info);
        _objectMapper = std::static_pointer_cast<ObjectMapper>(_mockObjectMapper);
        _mockDashboardSystemApiClient = std::make_shared<StrictMock<MockDashboardSystemApiClient>>(_objectMapper);
        _executor = std::make_shared<oatpp::async::Executor>(1, 1, 1);
        _mockBodyDecoder = std::make_shared<StrictMock<MockBodyDecoder>>();
        _nullInput = std::make_shared<MockInputStream>();

        EXPECT_CALL(_dummyLogger, GetLogLevel).WillRepeatedly(Return(Services::Logging::Level::Debug));
    }

    std::shared_ptr<IDashboardSystemServiceClient> CreateService()
    {
        return std::make_shared<DashboardSystemServiceClient>(&_dummyLogger, _mockDashboardSystemApiClient,
                                                              _objectMapper, _executor);
    }

    void WaitForExecutor()
    {
        _executor->waitTasksFinished();
        _executor->stop();
        _executor->join();
    }

    Core::Tests::MockLogger _dummyLogger;
    std::shared_ptr<StrictMock<MockDashboardSystemApiClient>> _mockDashboardSystemApiClient;
    std::shared_ptr<MockObjectMapper> _mockObjectMapper;
    std::shared_ptr<ObjectMapper> _objectMapper;
    std::shared_ptr<oatpp::async::Executor> _executor;
    std::shared_ptr<MockInputStream> _nullInput;
    std::shared_ptr<StrictMock<MockBodyDecoder>> _mockBodyDecoder;
    ObjectMapper::Info _info = "application/json";
};

TEST_F(TestDashboardSystemServiceClient, CreateSimulation_Success)
{
    // Arrange
    EXPECT_CALL(*_mockObjectMapper, write).Times(1);
    oatpp::String actualPath;
    oatpp::String actualMethod;
    EXPECT_CALL(*_mockDashboardSystemApiClient, executeRequestAsync)
        .WillOnce(DoAll(WithArgs<0, 1, 3>([&](auto currentMethod, auto pathTemplate, auto map) {
                            actualMethod = currentMethod;
                            actualPath = pathTemplate.format(map);
                        }),
                        Return(MockExecuteRequestAsync::startForResult(Status::CODE_201, _nullInput, _mockBodyDecoder))));
    EXPECT_CALL(*_mockBodyDecoder, decodeAsync).WillOnce(Return(MockDecodeAsync::start()));
    auto expectedResponse = SimulationCreationResponseDto::createShared();
    expectedResponse->id = 123;
    EXPECT_CALL(*_mockObjectMapper, read).WillOnce(Return(expectedResponse));
    EXPECT_CALL(_dummyLogger, Log(Services::Logging::Level::Debug, "Dashboard: created simulation with id 123"));

    // Act
    const auto service = CreateService();
    auto request = SimulationCreationRequestDto::createShared();
    auto res = service->CreateSimulation(request);
    WaitForExecutor();

    // Assert
    ASSERT_EQ(res.get(), expectedResponse);
    ASSERT_STREQ(actualMethod->c_str(), "POST");
    ASSERT_STREQ(actualPath->c_str(), "system-service/v1.0/simulations");
}

TEST_F(TestDashboardSystemServiceClient, CreateSimulation_Failure)
{
    // Arrange
    EXPECT_CALL(*_mockObjectMapper, write).Times(1);
    oatpp::String actualPath;
    oatpp::String actualMethod;
    EXPECT_CALL(*_mockDashboardSystemApiClient, executeRequestAsync)
        .WillOnce(
            DoAll(WithArgs<0, 1, 3>([&](auto currentMethod, auto pathTemplate, auto map) {
                      actualMethod = currentMethod;
                      actualPath = pathTemplate.format(map);
                  }),
                  Return(MockExecuteRequestAsync::startForResult(Status::CODE_500, _nullInput, _mockBodyDecoder))));
    EXPECT_CALL(_dummyLogger, Log(Services::Logging::Level::Error, "Dashboard: creating simulation failed: 500"));

    // Act
    const auto service = CreateService();
    auto request = SimulationCreationRequestDto::createShared();
    auto res = service->CreateSimulation(request);
    WaitForExecutor();

    // Assert
    ASSERT_FALSE(res.get());
    ASSERT_STREQ(actualMethod->c_str(), "POST");
    ASSERT_STREQ(actualPath->c_str(), "system-service/v1.0/simulations");
}

TEST_F(TestDashboardSystemServiceClient, SetSimulationEnd_Success)
{
    // Arrange
    EXPECT_CALL(*_mockObjectMapper, write).Times(1);
    oatpp::String actualPath;
    oatpp::String actualMethod;
    EXPECT_CALL(*_mockDashboardSystemApiClient, executeRequestAsync)
        .WillOnce(
            DoAll(WithArgs<0, 1, 3>([&](auto currentMethod, auto pathTemplate, auto map) {
                      actualMethod = currentMethod;
                      actualPath = pathTemplate.format(map);
                  }),
                  Return(MockExecuteRequestAsync::startForResult(Status::CODE_204, _nullInput, _mockBodyDecoder))));
    EXPECT_CALL(_dummyLogger, Log(Services::Logging::Level::Debug, "Dashboard: setting simulation end succeeded: 204"));

    // Act
    const auto service = CreateService();
    const oatpp::UInt64 expectedSimulationId = 123;
    auto request = SimulationEndDto::createShared();
    service->SetSimulationEnd(expectedSimulationId, request);
    WaitForExecutor();

    // Assert
    ASSERT_STREQ(actualMethod->c_str(), "POST");
    ASSERT_STREQ(actualPath->c_str(), "system-service/v1.0/simulations/123");
}

TEST_F(TestDashboardSystemServiceClient, SetSimulationEnd_Failure)
{
    // Arrange
    EXPECT_CALL(*_mockObjectMapper, write).Times(1);
    oatpp::String actualPath;
    oatpp::String actualMethod;
    EXPECT_CALL(*_mockDashboardSystemApiClient, executeRequestAsync)
        .WillOnce(
            DoAll(WithArgs<0, 1, 3>([&](auto currentMethod, auto pathTemplate, auto map) {
                      actualMethod = currentMethod;
                      actualPath = pathTemplate.format(map);
                  }),
                  Return(MockExecuteRequestAsync::startForResult(Status::CODE_500, _nullInput, _mockBodyDecoder))));
    EXPECT_CALL(_dummyLogger, Log(Services::Logging::Level::Error, "Dashboard: setting simulation end failed: 500"));

    // Act
    const auto service = CreateService();
    const oatpp::UInt64 expectedSimulationId = 123;
    auto request = SimulationEndDto::createShared();
    service->SetSimulationEnd(expectedSimulationId, request);
    WaitForExecutor();

    // Assert
    ASSERT_STREQ(actualMethod->c_str(), "POST");
    ASSERT_STREQ(actualPath->c_str(), "system-service/v1.0/simulations/123");
}

} // namespace Dashboard
} // namespace SilKit
