// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IVAsioPeer.hpp"

#include <vector>
#include <queue>
#include <mutex>

#include "asio.hpp"
#include "MessageBuffer.hpp"
#include "ib/mw/EndpointAddress.hpp"
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
    VAsioTcpPeer(asio::io_context& io_context, VAsioConnection* ibConnection);
    VAsioTcpPeer(const VAsioTcpPeer& other) = delete;
    VAsioTcpPeer(VAsioTcpPeer&& other) = default;

    VAsioTcpPeer& operator=(const VAsioTcpPeer& other) = delete;
    VAsioTcpPeer& operator=(VAsioTcpPeer&& other) = default;

public:
    // ----------------------------------------
    // Public Methods
    void SendIbMsg(MessageBuffer buffer) override;
    void Subscribe(VAsioMsgSubscriber subscriber) override;

    auto GetInfo() const -> const VAsioPeerInfo& override;
    void SetInfo(VAsioPeerInfo info) override;

    void Connect(VAsioPeerInfo info);

    inline auto Socket() -> asio::ip::tcp::socket& { return _socket; }

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

private:
    // ----------------------------------------
    // Private Members
    asio::ip::tcp::socket _socket;
    VAsioConnection* _ibConnection{nullptr};
    VAsioPeerInfo _info;

    // receiving
    uint32_t _currentMsgSize{0u};
    std::vector<uint8_t> _msgBuffer;
    size_t _wPos;

    // sending
    std::queue<std::vector<uint8_t>> _sendingQueue;
    asio::mutable_buffer _currentSendingBuffer;
    std::vector<uint8_t> _currentSendingBufferData;
    std::mutex _sendingQueueLock;
    std::size_t _sendingQueueMaxSize{100u};
    bool _sending{false};
};


// ================================================================================
//  Inline Implementations
// ================================================================================


} // mw
} // namespace ib
