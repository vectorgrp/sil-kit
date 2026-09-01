// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

// Test-only helper: a scripted HTTP server on an ephemeral loopback port.

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/read.hpp"
#include "asio/read_until.hpp"
#include "asio/streambuf.hpp"
#include "asio/write.hpp"

namespace VSilKit {
namespace Tests {

/*! A minimal scripted HTTP server for driving the dashboard's HTTP client.
 *
 *  The handler receives the full request (head plus body) and returns the raw bytes to reply with.
 *  Returning an empty string makes the server go silent instead, which is how the timeout and abort
 *  cases are driven.
 */
class FakeHttpServer
{
public:
    using Handler = std::function<std::string(const std::string& request)>;

    explicit FakeHttpServer(Handler handler, bool closeAfterReply = false)
        : _handler{std::move(handler)}
        , _closeAfterReply{closeAfterReply}
        , _acceptor{_ioContext, asio::ip::tcp::endpoint{asio::ip::make_address("127.0.0.1"), 0}}
    {
        _port = _acceptor.local_endpoint().port();
        _thread = std::thread{[this] { Serve(); }};
    }

    ~FakeHttpServer()
    {
        _stop = true;
        std::error_code ignored;
        _acceptor.close(ignored);
        if (_thread.joinable())
        {
            _thread.join();
        }
    }

    FakeHttpServer(const FakeHttpServer&) = delete;
    auto operator=(const FakeHttpServer&) -> FakeHttpServer& = delete;

    auto Port() const -> uint16_t
    {
        return _port;
    }

    auto AcceptCount() const -> int
    {
        return _acceptCount.load();
    }

    auto Requests() const -> std::vector<std::string>
    {
        std::lock_guard<std::mutex> lock{_mutex};
        return _requests;
    }

    //! Convenience: a well-formed response with a Content-Length body.
    static auto MakeReply(int statusCode, const std::string& body,
                          const std::string& extraHeaders = {}) -> std::string
    {
        return "HTTP/1.1 " + std::to_string(statusCode) + " Status\r\n" + extraHeaders + "Content-Length: "
               + std::to_string(body.size()) + "\r\n\r\n" + body;
    }

    //! Convenience: a handler that always answers with the same bytes.
    static auto Always(std::string reply) -> Handler
    {
        return [reply = std::move(reply)](const std::string&) { return reply; };
    }

private:
    void Serve()
    {
        while (!_stop)
        {
            asio::ip::tcp::socket socket{_ioContext};
            std::error_code ec;
            _acceptor.accept(socket, ec);
            if (ec)
            {
                return; // the destructor closed the acceptor
            }
            ++_acceptCount;
            ServeConnection(socket);
        }
    }

    void ServeConnection(asio::ip::tcp::socket& socket)
    {
        for (;;)
        {
            std::error_code ec;
            asio::streambuf buffer;
            const auto headSize = asio::read_until(socket, buffer, "\r\n\r\n", ec);
            if (ec || headSize == 0)
            {
                return;
            }

            std::string request{asio::buffer_cast<const char*>(buffer.data()), headSize};
            buffer.consume(headSize);

            size_t contentLength = 0;
            const auto pos = request.find("Content-Length: ");
            if (pos != std::string::npos)
            {
                contentLength = static_cast<size_t>(std::stoul(request.substr(pos + 16)));
            }
            if (contentLength > 0)
            {
                if (buffer.size() < contentLength)
                {
                    asio::read(socket, buffer, asio::transfer_exactly(contentLength - buffer.size()), ec);
                    if (ec)
                    {
                        return;
                    }
                }
                request.append(asio::buffer_cast<const char*>(buffer.data()), contentLength);
            }

            {
                std::lock_guard<std::mutex> lock{_mutex};
                _requests.push_back(request);
            }

            const auto reply = _handler(request);
            if (reply.empty())
            {
                // Go silent: the client must hit its read deadline, or be aborted.
                while (!_stop)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds{10});
                }
                return;
            }
            asio::write(socket, asio::buffer(reply), ec);
            if (ec || _closeAfterReply)
            {
                return;
            }
        }
    }

    Handler _handler;
    bool _closeAfterReply;
    asio::io_context _ioContext{1};
    asio::ip::tcp::acceptor _acceptor;
    uint16_t _port{0};
    std::atomic<bool> _stop{false};
    std::atomic<int> _acceptCount{0};
    mutable std::mutex _mutex;
    std::vector<std::string> _requests;
    std::thread _thread;
};

} // namespace Tests
} // namespace VSilKit
