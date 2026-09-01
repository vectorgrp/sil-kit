// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>

namespace VSilKit {

//! Outcome of a single HTTP exchange.
struct HttpResult
{
    //! True when no HTTP response was obtained at all (DNS, connect, write, read or parse failure,
    //! or an aborted client). Equivalent to oatpp's null response, which the dashboard logged as
    //! "server unavailable".
    bool transportError{true};

    //! HTTP status code; only meaningful when transportError is false.
    int statusCode{0};

    //! Response body; only populated when transportError is false.
    std::string body;
};

/*! A minimal blocking HTTP client, sufficient for the dashboard's three POST endpoints.
 *
 *  Implementations must never throw out of Post(): every failure is reported as
 *  HttpResult::transportError. Post() is expected to be called from a single thread; Abort() may be
 *  called concurrently from another.
 */
struct IHttpClient
{
    virtual ~IHttpClient() = default;

    //! POST a JSON body. `path` must not have a leading '/'.
    virtual auto Post(const std::string& path, const std::string& jsonBody) -> HttpResult = 0;

    //! Drop any cached connection. Equivalent to oatpp's invalidateConnection.
    virtual void Reset() = 0;

    /*! Unblock any in-flight Post() and make all subsequent calls fail fast.
     *
     *  Idempotent, and safe to call from a thread other than the one in Post(). Used on shutdown so
     *  a dashboard server that accepts connections but never answers cannot stall the registry.
     */
    virtual void Abort() = 0;
};

} // namespace VSilKit
