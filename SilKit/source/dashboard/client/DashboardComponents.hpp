// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "OatppHeaders.hpp"

namespace SilKit {
namespace Dashboard {

class DashboardComponents
{
private:
    std::string _host;
    uint16_t _port;

public:
    DashboardComponents(const std::string& host, uint16_t port)
        : _host(host)
        , _port(port)
    {
    }

public:
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ClientConnectionProvider>, clientConnectionProvider)
    ("DashboardComponents_clientConnectionProvider",
     [this]() -> std::shared_ptr<oatpp::network::ClientConnectionProvider> {
         auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared({_host, _port});
         return oatpp::network::ClientConnectionPool::createShared(connectionProvider, 5, std::chrono::seconds(10),
                                                                   std::chrono::seconds(5));
     }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)
    ("DashboardComponents_apiObjectMapper", [] {
        auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
        objectMapper->getDeserializer()->getConfig()->allowUnknownFields = false;
        return objectMapper;
    }());
};

} // namespace Dashboard
} // namespace SilKit
