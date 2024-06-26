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
