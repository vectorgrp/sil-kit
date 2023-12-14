// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "IRawByteStream.hpp"

#include "AsioSocketOptions.hpp"
#include "util/Atomic.hpp"

#include "ILogger.hpp"

#include <memory>
#include <mutex>

#include "asio.hpp"


namespace VSilKit {


struct AsioGenericRawByteStreamOptions
{
    struct
    {
        bool quickAck{false};
    } tcp;
};


class AsioGenericRawByteStream final : public IRawByteStream
{
    using AsioSocket = asio::generic::stream_protocol::socket;

    IRawByteStreamListener* _listener{nullptr};

    std::mutex _mutex;
    bool _shutdownPending{false};
    bool _shutdownPosted{false};
    bool _reading{false};
    bool _writing{false};

    std::vector<asio::mutable_buffer> _readBufferSequence;
    std::vector<asio::const_buffer> _writeBufferSequence;

    AsioGenericRawByteStreamOptions _options;

    std::shared_ptr<asio::io_context> _asioIoContext;
    AsioSocket _socket;

    SilKit::Services::Logging::ILogger* _logger{nullptr};

public:
    AsioGenericRawByteStream(const AsioGenericRawByteStreamOptions& options,
                             std::shared_ptr<asio::io_context> asioIoContext, AsioSocket socket,
                             SilKit::Services::Logging::ILogger& logger);
    ~AsioGenericRawByteStream() override;

public: // IRawByteStream
    void SetListener(IRawByteStreamListener& listener) override;
    auto GetLocalEndpoint() const -> std::string override;
    auto GetRemoteEndpoint() const -> std::string override;
    void AsyncReadSome(MutableBufferSequence bufferSequence) override;
    void AsyncWriteSome(ConstBufferSequence bufferSequence) override;
    void Shutdown() override;

private:
    void OnAsioAsyncReadSomeComplete(const asio::error_code& errorCode, size_t bytesTransferred);
    void OnAsioAsyncWriteSomeComplete(const asio::error_code& errorCode, size_t bytesTransferred);

    void HandleShutdownOrError();

private:
    void EnableQuickAck();
};


} // namespace VSilKit
