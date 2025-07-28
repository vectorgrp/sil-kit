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

#pragma once

#include "gmock/gmock-function-mocker.h"

#include "DashboardSystemApiClient.hpp"

namespace SilKit {
namespace Dashboard {
class MockDashboardSystemApiClient : public DashboardSystemApiClient
{
    using RequestExecutor = oatpp::web::client::RequestExecutor;
    using ObjectMapper = oatpp::data::mapping::ObjectMapper;

public:
    MockDashboardSystemApiClient(const std::shared_ptr<ObjectMapper>& objectMapper)
        : DashboardSystemApiClient(std::shared_ptr<RequestExecutor>(nullptr), objectMapper)
    {
    }

    MOCK_METHOD(std::shared_ptr<RequestExecutor::ConnectionHandle>, getConnection, (), (override));
    MOCK_METHOD(oatpp::async::CoroutineStarterForResult<const std::shared_ptr<RequestExecutor::ConnectionHandle>&>,
                getConnectionAsync, (), (override));
    MOCK_METHOD(std::shared_ptr<Response>, executeRequest,
                (const oatpp::String&, const StringTemplate&, const Headers&,
                 (const std::unordered_map<oatpp::String, oatpp::String>&),
                 (const std::unordered_map<oatpp::String, oatpp::String>&),
                 const std::shared_ptr<RequestExecutor::Body>&,
                 const std::shared_ptr<RequestExecutor::ConnectionHandle>&),
                (override));
    MOCK_METHOD(oatpp::async::CoroutineStarterForResult<const std::shared_ptr<Response>&>, executeRequestAsync,
                (const oatpp::String&, const StringTemplate&, const Headers&,
                 (const std::unordered_map<oatpp::String, oatpp::String>&),
                 (const std::unordered_map<oatpp::String, oatpp::String>&),
                 const std::shared_ptr<RequestExecutor::Body>&,
                 const std::shared_ptr<RequestExecutor::ConnectionHandle>&),
                (override));
};
} // namespace Dashboard
} // namespace SilKit
