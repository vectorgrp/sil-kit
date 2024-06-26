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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#endif

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

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

class Test_DashboardSystemServiceClient : public Test
{
public:
    void SetUp() override
    {
        _mockObjectMapper = std::make_shared<MockObjectMapper>(_info);
        _objectMapper = std::static_pointer_cast<ObjectMapper>(_mockObjectMapper);
        _mockDashboardSystemApiClient = std::make_shared<StrictMock<MockDashboardSystemApiClient>>(_objectMapper);
        _mockBodyDecoder = std::make_shared<StrictMock<MockBodyDecoder>>();
        _nullInput = std::make_shared<MockInputStream>();

        EXPECT_CALL(_dummyLogger, GetLogLevel).WillRepeatedly(Return(Services::Logging::Level::Debug));
    }

    std::shared_ptr<IDashboardSystemServiceClient> CreateService()
    {
        return std::make_shared<DashboardSystemServiceClient>(&_dummyLogger, _mockDashboardSystemApiClient,
                                                              _objectMapper);
    }

    void SetupExecuteRequest(
        Status status,
        const std::function<void(const oatpp::String&, const oatpp::data::share::StringTemplate&,
                                 const std::unordered_map<oatpp::String, oatpp::String>& pathParams)>& OnRequest)
    {
        std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response =
            oatpp::web::protocol::http::incoming::Response::createShared(status.code, status.description, Headers(),
                                                                         _nullInput, _mockBodyDecoder);
        EXPECT_CALL(*_mockDashboardSystemApiClient, executeRequest)
            .WillOnce(DoAll(WithArgs<0, 1, 3>([OnRequest](auto currentMethod, auto pathTemplate, auto map) {
            OnRequest(currentMethod, pathTemplate, map);
        }),
                            Return(response)));
    }

    Core::Tests::MockLogger _dummyLogger;
    std::shared_ptr<StrictMock<MockDashboardSystemApiClient>> _mockDashboardSystemApiClient;
    std::shared_ptr<MockObjectMapper> _mockObjectMapper;
    std::shared_ptr<ObjectMapper> _objectMapper;
    std::shared_ptr<MockInputStream> _nullInput;
    std::shared_ptr<StrictMock<MockBodyDecoder>> _mockBodyDecoder;
    ObjectMapper::Info _info = "application/json";
};

TEST_F(Test_DashboardSystemServiceClient, CreateSimulation_Success)
{
    // Arrange
    EXPECT_CALL(*_mockObjectMapper, write);
    oatpp::String actualPath;
    oatpp::String actualMethod;
    SetupExecuteRequest(Status::CODE_201,
                        [&actualPath, &actualMethod](auto currentMethod, auto pathTemplate, auto map) {
        actualMethod = currentMethod;
        actualPath = pathTemplate.format(map);
    });
    EXPECT_CALL(*_mockBodyDecoder, decode);
    auto expectedResponse = SimulationCreationResponseDto::createShared();
    expectedResponse->id = 123;
    EXPECT_CALL(*_mockObjectMapper, read).WillOnce(Return(expectedResponse));
    EXPECT_CALL(_dummyLogger, Log(Services::Logging::Level::Debug, "Dashboard: creating simulation returned 201"));

    // Act
    oatpp::Object<SimulationCreationResponseDto> response;
    {
        const auto service = CreateService();
        auto request = SimulationCreationRequestDto::createShared();
        response = service->CreateSimulation(request);
    }

    // Assert
    ASSERT_EQ(response, expectedResponse);
    ASSERT_STREQ(actualMethod->c_str(), "POST");
    ASSERT_STREQ(actualPath->c_str(), "system-service/v1.0/simulations");
}

TEST_F(Test_DashboardSystemServiceClient, CreateSimulation_Failure)
{
    // Arrange
    EXPECT_CALL(*_mockObjectMapper, write);
    oatpp::String actualPath;
    oatpp::String actualMethod;
    SetupExecuteRequest(Status::CODE_500,
                        [&actualPath, &actualMethod](auto currentMethod, auto pathTemplate, auto map) {
        actualMethod = currentMethod;
        actualPath = pathTemplate.format(map);
    });
    EXPECT_CALL(_dummyLogger, Log(Services::Logging::Level::Error, "Dashboard: creating simulation returned 500"));

    // Act
    oatpp::Object<SimulationCreationResponseDto> response;
    {
        const auto service = CreateService();
        auto request = SimulationCreationRequestDto::createShared();
        service->CreateSimulation(request);
    }

    // Assert
    ASSERT_TRUE(response == nullptr);
    ASSERT_STREQ(actualMethod->c_str(), "POST");
    ASSERT_STREQ(actualPath->c_str(), "system-service/v1.0/simulations");
}

TEST_F(Test_DashboardSystemServiceClient, SetSimulationEnd_Success)
{
    // Arrange
    EXPECT_CALL(*_mockObjectMapper, write);
    oatpp::String actualPath;
    oatpp::String actualMethod;
    SetupExecuteRequest(Status::CODE_204,
                        [&actualPath, &actualMethod](auto currentMethod, auto pathTemplate, auto map) {
        actualMethod = currentMethod;
        actualPath = pathTemplate.format(map);
    });
    EXPECT_CALL(_dummyLogger, Log(Services::Logging::Level::Debug, "Dashboard: setting simulation end returned 204"));

    // Act
    {
        const auto service = CreateService();
        const oatpp::UInt64 expectedSimulationId = 123;
        auto request = SimulationEndDto::createShared();
        service->SetSimulationEnd(expectedSimulationId, request);
    }

    // Assert
    ASSERT_STREQ(actualMethod->c_str(), "POST");
    ASSERT_STREQ(actualPath->c_str(), "system-service/v1.0/simulations/123");
}

TEST_F(Test_DashboardSystemServiceClient, SetSimulationEnd_Failure)
{
    // Arrange
    EXPECT_CALL(*_mockObjectMapper, write);
    oatpp::String actualPath;
    oatpp::String actualMethod;
    SetupExecuteRequest(Status::CODE_500,
                        [&actualPath, &actualMethod](auto currentMethod, auto pathTemplate, auto map) {
        actualMethod = currentMethod;
        actualPath = pathTemplate.format(map);
    });
    EXPECT_CALL(_dummyLogger, Log(Services::Logging::Level::Error, "Dashboard: setting simulation end returned 500"));

    // Act
    {
        const auto service = CreateService();
        const oatpp::UInt64 expectedSimulationId = 123;
        auto request = SimulationEndDto::createShared();
        service->SetSimulationEnd(expectedSimulationId, request);
    }

    // Assert
    ASSERT_STREQ(actualMethod->c_str(), "POST");
    ASSERT_STREQ(actualPath->c_str(), "system-service/v1.0/simulations/123");
}

} // namespace Dashboard
} // namespace SilKit
