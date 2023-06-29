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


#include <vector>
#include <queue>
#include <mutex>
#include <sstream>

#include "asio.hpp"

#include "silkit/services/logging/ILogger.hpp"

#include "EndpointAddress.hpp"
#include "MessageBuffer.hpp"
#include "VAsioPeerInfo.hpp"
#include "ProtocolVersion.hpp"
#include "IVAsioConnectionPeer.hpp"


namespace SilKit {
namespace Core {

class VAsioConnection;

class VAsioTcpPeer
    : public IVAsioConnectionPeer
    , public std::enable_shared_from_this<VAsioTcpPeer>
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    VAsioTcpPeer() = delete;
    VAsioTcpPeer(const VAsioTcpPeer& other) = delete;
    VAsioTcpPeer(VAsioTcpPeer&& other) = delete; //clang warning: implicitly deleted because of mutex

    VAsioTcpPeer& operator=(const VAsioTcpPeer& other) = delete;
    VAsioTcpPeer& operator=(VAsioTcpPeer&& other) = delete; //implicitly deleted because of mutex
    ~VAsioTcpPeer();

private:
    // ----------------------------------------
    // Private Constructors
    VAsioTcpPeer(asio::any_io_executor executor, VAsioConnection* connection, Services::Logging::ILogger* logger);

public:
    // ----------------------------------------
    // Public Construction Function

    // VAsioTcpPeer must only be created as shared_prt to keep it alive in active Read/WriteSomeAsync callbacks during shutdown procedure
    template<typename ExecutorT>
    static auto Create(ExecutorT&& executor, VAsioConnection* connection,
                              Services::Logging::ILogger* logger) -> std::shared_ptr<VAsioTcpPeer>
    {
        return std::shared_ptr<VAsioTcpPeer>{new VAsioTcpPeer{std::forward<ExecutorT>(executor), connection, logger}};
    }

public:
    // ----------------------------------------
    // Public Methods
    void SendSilKitMsg(SerializedMessage buffer) override;
    void Subscribe(VAsioMsgSubscriber subscriber) override;

    auto GetInfo() const -> const VAsioPeerInfo& override;
    void SetInfo(VAsioPeerInfo info) override;
    //!< Return the socket address as URI encoded string or throw if not connected
    auto GetRemoteAddress() const -> std::string override;
    auto GetLocalAddress() const -> std::string override;

    void Connect(VAsioPeerInfo info,
                 std::stringstream& attemptedUris,
                 bool& success);

    inline auto Socket() -> asio::generic::stream_protocol::socket& { return _socket; }

    void StartAsyncRead() override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;

    inline void SetProtocolVersion(ProtocolVersion v)  override;
    inline auto GetProtocolVersion() const -> ProtocolVersion  override;
    
    void DrainAllBuffers() override;

private:
    // ----------------------------------------
    // Private Methods
    static bool IsErrorToTryAgain(const asio::error_code & ec);
    void StartAsyncWrite();
    void WriteSomeAsync();
    void ReadSomeAsync();
    void DispatchBuffer();
    void Shutdown();
    bool ConnectLocal(const std::string& path);
    void ResetSocket();
    void ResetTcpSocket(const std::string& host, uint16_t port, const std::string& message);
    bool ConnectTcp(const std::string& host, uint16_t port);

private:
    // ----------------------------------------
    // Private Members
    ProtocolVersion _protocolVersion{};
    asio::generic::stream_protocol::socket _socket;
    VAsioConnection* _connection{nullptr};
    VAsioPeerInfo _info;

    Services::Logging::ILogger* _logger;

    // receiving
    std::atomic<uint32_t> _currentMsgSize{0u};
    std::vector<uint8_t> _msgBuffer;
    size_t _wPos{0};

    // sending
    std::atomic_bool _isShuttingDown{false};
    std::deque<std::vector<uint8_t>> _sendingQueue;
    asio::mutable_buffer _currentSendingBuffer;
    std::vector<uint8_t> _currentSendingBufferData;
    mutable std::mutex _sendingQueueLock;
    std::atomic_bool _sending{false};
    bool _enableQuickAck{false};
    Core::ServiceDescriptor _serviceDescriptor;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void VAsioTcpPeer::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}
auto VAsioTcpPeer::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

void VAsioTcpPeer::SetProtocolVersion(ProtocolVersion v)
{
    _protocolVersion = std::move(v);
}

auto VAsioTcpPeer::GetProtocolVersion() const -> ProtocolVersion
{
    return _protocolVersion;
}

} // namespace Core
} // namespace SilKit
