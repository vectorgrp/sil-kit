// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IVAsioPeer.hpp"

#include <vector>
#include <queue>
#include <mutex>

#include "silkit/services/logging/ILogger.hpp"

#include "EndpointAddress.hpp"
#include "MessageBuffer.hpp"
#include "VAsioPeerInfo.hpp"
#include "IServiceEndpoint.hpp"
#include "ProtocolVersion.hpp"

#include "asio.hpp"

namespace SilKit {
namespace Core {

class VAsioConnection;

class VAsioTcpPeer
    : public IVAsioPeer
    , public IServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    VAsioTcpPeer() = delete;
    VAsioTcpPeer(asio::any_io_executor executor, VAsioConnection* connection, Services::Logging::ILogger* logger);
    VAsioTcpPeer(const VAsioTcpPeer& other) = delete;
    VAsioTcpPeer(VAsioTcpPeer&& other) = delete; //clang warning: implicitly deleted because of mutex

    VAsioTcpPeer& operator=(const VAsioTcpPeer& other) = delete;
    VAsioTcpPeer& operator=(VAsioTcpPeer&& other) = delete; //implicitly deleted because of mutex

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

    void Connect(VAsioPeerInfo info);

    inline auto Socket() -> asio::generic::stream_protocol::socket& { return _socket; }

    void StartAsyncRead() override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;

    inline void SetProtocolVersion(ProtocolVersion v)  override;
    inline auto GetProtocolVersion() const -> ProtocolVersion  override;
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
    uint32_t _currentMsgSize{0u};
    std::vector<uint8_t> _msgBuffer;
    size_t _wPos{0};

    // sending
    std::queue<std::vector<uint8_t>> _sendingQueue;
    asio::mutable_buffer _currentSendingBuffer;
    std::vector<uint8_t> _currentSendingBufferData;
    std::mutex _sendingQueueLock;
    bool _sending{false};
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
