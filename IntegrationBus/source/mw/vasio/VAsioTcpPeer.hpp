// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IVAsioPeer.hpp"

#include <vector>
#include <queue>
#include <mutex>

#include "asio.hpp"
#include "MessageBuffer.hpp"
#include "ib/mw/EndpointAddress.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "VAsioPeerInfo.hpp"

namespace ib {
namespace mw {

class VAsioConnection;

class VAsioTcpPeer : public IVAsioPeer
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    VAsioTcpPeer() = delete;
    VAsioTcpPeer(asio::any_io_executor executor, VAsioConnection* ibConnection, logging::ILogger* logger);
    VAsioTcpPeer(const VAsioTcpPeer& other) = delete;
    VAsioTcpPeer(VAsioTcpPeer&& other) = delete; //clang warning: implicitly deleted because of mutex

    VAsioTcpPeer& operator=(const VAsioTcpPeer& other) = delete;
    VAsioTcpPeer& operator=(VAsioTcpPeer&& other) = delete; //implicitly deleted because of mutex

public:
    // ----------------------------------------
    // Public Methods
    void SendIbMsg(MessageBuffer buffer) override;
    void Subscribe(VAsioMsgSubscriber subscriber) override;

    auto GetInfo() const -> const VAsioPeerInfo& override;
    void SetInfo(VAsioPeerInfo info) override;

    void Connect(VAsioPeerUri info);

    inline auto Socket() -> asio::generic::stream_protocol::socket& { return _socket; }

    void StartAsyncRead();

private:
    // ----------------------------------------
    // Private Methods
    static bool IsErrorToTryAgain(const asio::error_code & ec);
    void StartAsyncWrite();
    void WriteSomeAsync();
    void ReadSomeAsync();
    void DispatchBuffer();
    void Shutdown();
    void ConnectLocal(const std::string& socketPath);
    void ConnectTcp(const std::string& host, uint16_t port);

private:
    // ----------------------------------------
    // Private Members
    asio::generic::stream_protocol::socket _socket;
    VAsioConnection* _ibConnection{nullptr};
    VAsioPeerInfo _info;
    VAsioPeerUri _uri;

    logging::ILogger* _logger;

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
};


// ================================================================================
//  Inline Implementations
// ================================================================================


} // mw
} // namespace ib
