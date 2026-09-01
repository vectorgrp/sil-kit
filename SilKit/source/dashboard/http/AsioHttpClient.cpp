// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "dashboard/http/AsioHttpClient.hpp"

#include <algorithm>
#include <optional>
#include <utility>

#include "asio/connect.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/post.hpp"
#include "asio/read.hpp"
#include "asio/read_until.hpp"
#include "asio/streambuf.hpp"
#include "asio/write.hpp"

#include "core/internal/traits/SilKitLoggingTraits.hpp"
#include "services/logging/LoggerMessage.hpp"

using SilKit::Services::Logging::Level;
using SilKit::Services::Logging::LoggerMessage;

namespace VSilKit {

namespace {

//! Sentinel no asio operation ever completes with, so it doubles as "still pending".
const std::error_code kPending = asio::error::would_block;

//! How often the deadline loop wakes to notice an Abort(); also bounds abort latency.
constexpr auto kPollInterval = std::chrono::milliseconds{50};

constexpr size_t kMaxHeadSize = 64 * 1024;

} // namespace

struct AsioHttpClient::Impl
{
    SilKit::Services::Logging::ILoggerInternal* logger;
    std::string host;
    uint16_t port;
    std::string hostHeader;
    AsioHttpClientTimeouts timeouts;

    asio::io_context ioContext{1};
    std::optional<asio::ip::tcp::socket> socket;
    asio::streambuf readBuffer;
    std::vector<asio::ip::tcp::endpoint> endpoints;
    std::chrono::steady_clock::time_point lastUse{};
    std::atomic<bool> aborted{false};

    Impl(SilKit::Services::Logging::ILoggerInternal* logger_, std::string host_, uint16_t port_,
         AsioHttpClientTimeouts timeouts_)
        : logger{logger_}
        , host{std::move(host_)}
        , port{port_}
        , hostHeader{host + ":" + std::to_string(port_)}
        , timeouts{timeouts_}
    {
    }

    void Log(Level level, const std::string& message) const
    {
        if (logger == nullptr)
        {
            return;
        }
        logger->MakeMessage(level, SilKit::Core::SilKitTopicTrait<AsioHttpClient>::Topic())
            .SetMessage("Dashboard: {}", message)
            .Dispatch();
    }

    void DropSocket()
    {
        if (socket.has_value())
        {
            std::error_code ignored;
            socket->cancel(ignored);
            socket->close(ignored);
            socket.reset();
        }
        readBuffer.consume(readBuffer.size());
    }

    /*! Drive one async operation to completion under a deadline.
     *
     *  The call site is synchronous, so instead of a timer we run the io_context in short slices and
     *  check the deadline and the abort flag between them.
     */
    template <typename Initiate>
    auto RunWithDeadline(Initiate&& initiate, std::chrono::milliseconds timeout) -> std::error_code
    {
        auto opError = kPending;
        initiate([&opError](const std::error_code& ec) { opError = ec; });

        ioContext.restart();
        const auto deadline = std::chrono::steady_clock::now() + timeout;

        while (opError == kPending)
        {
            if (aborted.load(std::memory_order_acquire))
            {
                return DrainAndFail(asio::error::operation_aborted);
            }
            const auto remaining = deadline - std::chrono::steady_clock::now();
            if (remaining <= std::chrono::steady_clock::duration::zero())
            {
                return DrainAndFail(asio::error::timed_out);
            }
            ioContext.run_one_for(std::min<std::chrono::steady_clock::duration>(remaining, kPollInterval));
        }
        return opError;
    }

    /*! Cancel the pending operation and let its handler run.
     *
     *  Without draining, the handler would later write through a dangling reference to the caller's
     *  stack-allocated error_code.
     */
    auto DrainAndFail(std::error_code reason) -> std::error_code
    {
        DropSocket();
        ioContext.restart();
        ioContext.run();
        return reason;
    }

    auto Resolve() -> std::error_code
    {
        if (!endpoints.empty())
        {
            return {};
        }
        // Resolved once and cached: the dashboard URI never changes, and a hung getaddrinfo cannot
        // be reliably cancelled, so we want to be exposed to it at most once.
        asio::ip::tcp::resolver resolver{ioContext};
        asio::ip::tcp::resolver::results_type results;
        const auto ec = RunWithDeadline(
            [&](auto handler) {
                resolver.async_resolve(host, std::to_string(port),
                                       [handler, &results](const std::error_code& e,
                                                           asio::ip::tcp::resolver::results_type r) {
            if (!e)
            {
                results = std::move(r);
            }
            handler(e);
                                       });
            },
            timeouts.connect);
        if (ec)
        {
            return ec;
        }
        for (const auto& entry : results)
        {
            endpoints.push_back(entry.endpoint());
        }
        if (endpoints.empty())
        {
            return asio::error::host_not_found;
        }
        return {};
    }

    //! Ensures a usable socket. `reused` reports whether an existing connection was kept.
    auto EnsureConnected(bool& reused) -> std::error_code
    {
        reused = false;

        if (socket.has_value())
        {
            const auto age = std::chrono::steady_clock::now() - lastUse;
            if (age > timeouts.idle)
            {
                DropSocket(); // likely already closed by the server's idle timeout
            }
            else
            {
                reused = true;
                return {};
            }
        }

        if (const auto ec = Resolve())
        {
            return ec;
        }

        socket.emplace(ioContext);
        const auto ec = RunWithDeadline(
            [&](auto handler) {
                asio::async_connect(*socket, endpoints,
                                    [handler](const std::error_code& e, const asio::ip::tcp::endpoint&) {
            handler(e);
                                    });
            },
            timeouts.connect);
        if (ec)
        {
            DropSocket();
            return ec;
        }

        std::error_code ignored;
        socket->set_option(asio::ip::tcp::no_delay{true}, ignored); // small request/response exchanges

        return {};
    }

    auto BuildRequest(const std::string& path, const std::string& body) const -> std::string
    {
        // The same header set oatpp sent, so the dashboard sees an equivalent request.
        std::string request;
        request.reserve(body.size() + 256);
        request += "POST /";
        request += path;
        request += " HTTP/1.1\r\nHost: ";
        request += hostHeader;
        request += "\r\nConnection: keep-alive\r\nContent-Type: application/json\r\nContent-Length: ";
        request += std::to_string(body.size());
        request += "\r\n\r\n";
        request += body;
        return request;
    }

    auto WriteAll(const std::string& request) -> std::error_code
    {
        return RunWithDeadline(
            [&](auto handler) {
                asio::async_write(*socket, asio::buffer(request),
                                  [handler](const std::error_code& e, size_t) { handler(e); });
            },
            timeouts.write);
    }

    //! Reads up to and including the blank line terminating the response head.
    auto ReadHead(std::string& head) -> std::error_code
    {
        size_t headSize = 0;
        const auto ec = RunWithDeadline(
            [&](auto handler) {
                asio::async_read_until(*socket, readBuffer, "\r\n\r\n",
                                       [handler, &headSize](const std::error_code& e, size_t n) {
            headSize = n;
            handler(e);
                                       });
            },
            timeouts.read);
        if (ec)
        {
            return ec;
        }
        if (headSize > kMaxHeadSize)
        {
            return asio::error::message_size;
        }
        const auto* data = asio::buffer_cast<const char*>(readBuffer.data());
        head.assign(data, headSize);
        readBuffer.consume(headSize);
        return {};
    }

    //! Reads exactly `count` bytes of body, serving from the buffer first.
    auto ReadExactly(uint64_t count, std::string& out) -> std::error_code
    {
        if (count > kMaxHttpBodySize)
        {
            return asio::error::message_size;
        }
        const auto wanted = static_cast<size_t>(count);
        if (readBuffer.size() < wanted)
        {
            const auto missing = wanted - readBuffer.size();
            const auto ec = RunWithDeadline(
                [&](auto handler) {
                    asio::async_read(*socket, readBuffer, asio::transfer_exactly(missing),
                                     [handler](const std::error_code& e, size_t) { handler(e); });
                },
                timeouts.read);
            if (ec)
            {
                return ec;
            }
        }
        const auto* data = asio::buffer_cast<const char*>(readBuffer.data());
        out.append(data, wanted);
        readBuffer.consume(wanted);
        return {};
    }

    //! Reads one CRLF-terminated line, serving from the buffer first.
    auto ReadLine(std::string& line) -> std::error_code
    {
        size_t lineSize = 0;
        const auto ec = RunWithDeadline(
            [&](auto handler) {
                asio::async_read_until(*socket, readBuffer, "\r\n",
                                       [handler, &lineSize](const std::error_code& e, size_t n) {
            lineSize = n;
            handler(e);
                                       });
            },
            timeouts.read);
        if (ec)
        {
            return ec;
        }
        const auto* data = asio::buffer_cast<const char*>(readBuffer.data());
        line.assign(data, lineSize >= 2 ? lineSize - 2 : 0); // strip CRLF
        readBuffer.consume(lineSize);
        return {};
    }

    auto ReadChunkedBody(std::string& out) -> std::error_code
    {
        for (;;)
        {
            std::string sizeLine;
            if (const auto ec = ReadLine(sizeLine))
            {
                return ec;
            }
            uint64_t chunkSize = 0;
            if (!ParseChunkSize(sizeLine, chunkSize))
            {
                return asio::error::invalid_argument;
            }
            if (chunkSize == 0)
            {
                break;
            }
            if (out.size() + chunkSize > kMaxHttpBodySize)
            {
                return asio::error::message_size;
            }
            if (const auto ec = ReadExactly(chunkSize, out))
            {
                return ec;
            }
            std::string crlf;
            if (const auto ec = ReadLine(crlf)) // the CRLF after the chunk data
            {
                return ec;
            }
            if (!crlf.empty())
            {
                return asio::error::invalid_argument;
            }
        }
        // Consume trailers up to the terminating blank line.
        for (;;)
        {
            std::string trailer;
            if (const auto ec = ReadLine(trailer))
            {
                return ec;
            }
            if (trailer.empty())
            {
                return {};
            }
        }
    }

    auto ReadUntilClose(std::string& out) -> std::error_code
    {
        const auto ec = RunWithDeadline(
            [&](auto handler) {
                asio::async_read(*socket, readBuffer,
                                 [handler](const std::error_code& e, size_t) { handler(e); });
            },
            timeouts.read);
        if (ec && ec != asio::error::eof)
        {
            return ec;
        }
        if (readBuffer.size() > kMaxHttpBodySize)
        {
            return asio::error::message_size;
        }
        const auto* data = asio::buffer_cast<const char*>(readBuffer.data());
        out.append(data, readBuffer.size());
        readBuffer.consume(readBuffer.size());
        return {};
    }

    //! Reads a full response. `receivedAnything` distinguishes a stale keep-alive from a real error.
    auto ReadResponse(ResponseHead& head, HttpResult& result, bool& receivedAnything) -> std::error_code
    {
        for (;;) // loop to skip 1xx interim responses
        {
            std::string rawHead;
            if (const auto ec = ReadHead(rawHead))
            {
                return ec;
            }
            receivedAnything = true;

            if (!ParseResponseHead(rawHead, head))
            {
                return asio::error::invalid_argument;
            }
            if (!head.interim)
            {
                break;
            }
        }

        switch (head.framing)
        {
        case HttpBodyFraming::None:
            break;
        case HttpBodyFraming::ContentLength:
            if (const auto ec = ReadExactly(head.contentLength, result.body))
            {
                return ec;
            }
            break;
        case HttpBodyFraming::Chunked:
            if (const auto ec = ReadChunkedBody(result.body))
            {
                return ec;
            }
            break;
        case HttpBodyFraming::UntilClose:
            if (const auto ec = ReadUntilClose(result.body))
            {
                return ec;
            }
            // The framing relies on the close, so the socket cannot be reused.
            head.connectionClose = true;
            break;
        }
        return {};
    }
};

AsioHttpClient::AsioHttpClient(SilKit::Services::Logging::ILoggerInternal* logger, std::string host, uint16_t port,
                               AsioHttpClientTimeouts timeouts)
    : _impl{std::make_unique<Impl>(logger, std::move(host), port, timeouts)}
{
}

AsioHttpClient::~AsioHttpClient()
{
    _impl->aborted.store(true, std::memory_order_release);
    _impl->DropSocket();
}

auto AsioHttpClient::Post(const std::string& path, const std::string& jsonBody) -> HttpResult
try
{
    if (_impl->aborted.load(std::memory_order_acquire))
    {
        return HttpResult{};
    }

    // Two passes at most: a reused socket may have been closed by the server's idle timeout
    // between requests, which must not surface as "server unavailable".
    for (int attempt = 0; attempt < 2; ++attempt)
    {
        bool reused = false;
        if (const auto ec = _impl->EnsureConnected(reused))
        {
            _impl->Log(Level::Debug, "connect to " + _impl->hostHeader + " failed: " + ec.message());
            return HttpResult{};
        }

        const auto request = _impl->BuildRequest(path, jsonBody);
        if (const auto ec = _impl->WriteAll(request))
        {
            _impl->DropSocket();
            if (reused && attempt == 0 && !_impl->aborted.load(std::memory_order_acquire))
            {
                continue; // stale keep-alive: reconnect and try once more
            }
            _impl->Log(Level::Debug, "sending request failed: " + ec.message());
            return HttpResult{};
        }

        ResponseHead head{};
        HttpResult result{};
        bool receivedAnything = false;
        if (const auto ec = _impl->ReadResponse(head, result, receivedAnything))
        {
            _impl->DropSocket();
            if (reused && !receivedAnything && attempt == 0 && !_impl->aborted.load(std::memory_order_acquire))
            {
                continue; // stale keep-alive
            }
            _impl->Log(Level::Debug, "reading response failed: " + ec.message());
            return HttpResult{};
        }

        if (head.connectionClose)
        {
            _impl->DropSocket();
        }
        else
        {
            _impl->lastUse = std::chrono::steady_clock::now();
        }

        result.transportError = false;
        result.statusCode = head.statusCode;
        return result;
    }
    return HttpResult{};
}
catch (const std::exception& e)
{
    // IHttpClient::Post must never throw; the dashboard worker treats this as "server unavailable".
    _impl->DropSocket();
    _impl->Log(Level::Debug, std::string{"HTTP request failed: "} + e.what());
    return HttpResult{};
}
catch (...)
{
    _impl->DropSocket();
    return HttpResult{};
}

void AsioHttpClient::Reset()
{
    _impl->DropSocket();
}

void AsioHttpClient::Abort()
{
    _impl->aborted.store(true, std::memory_order_release);
    // asio sockets are not thread-safe, so the close must happen on the io_context; posting it also
    // wakes a run_one_for() that is currently blocked.
    asio::post(_impl->ioContext, [impl = _impl.get()] { impl->DropSocket(); });
}

} // namespace VSilKit
