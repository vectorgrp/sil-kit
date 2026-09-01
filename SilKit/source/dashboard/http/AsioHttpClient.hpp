// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include "dashboard/http/HttpResponseParser.hpp"
#include "dashboard/http/IHttpClient.hpp"

#include "services/logging/ILoggerInternal.hpp"

// asio types are kept out of this header so that the 22k-line asio headers are confined to the
// single translation unit that implements the client.
namespace asio {
class io_context;
}

namespace VSilKit {

struct AsioHttpClientTimeouts
{
    std::chrono::milliseconds connect{5000};
    std::chrono::milliseconds write{5000};
    std::chrono::milliseconds read{30000};
    //! A cached connection older than this is dropped rather than reused.
    std::chrono::milliseconds idle{10000};
};

/*! A blocking HTTP/1.1 client over standalone asio, sized for the dashboard's three POSTs.
 *
 *  Only one thread (the dashboard's event-queue worker) ever calls Post(), so a single keep-alive
 *  socket replaces the five-connection pool oatpp used. Unlike oatpp, every step has a deadline:
 *  oatpp set no socket timeouts at all, which let an unresponsive dashboard stall the registry.
 */
class AsioHttpClient final : public IHttpClient
{
public:
    AsioHttpClient(SilKit::Services::Logging::ILoggerInternal* logger, std::string host, uint16_t port,
                   AsioHttpClientTimeouts timeouts = {});
    ~AsioHttpClient() override;

    AsioHttpClient(const AsioHttpClient&) = delete;
    auto operator=(const AsioHttpClient&) -> AsioHttpClient& = delete;

    auto Post(const std::string& path, const std::string& jsonBody) -> HttpResult override;
    void Reset() override;
    void Abort() override;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace VSilKit
