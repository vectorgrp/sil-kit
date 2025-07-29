// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
