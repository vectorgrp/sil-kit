// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include <array>
#include <deque>
#include <vector>
#include <queue>
#include <mutex>
#include <sstream>

#include "silkit/services/logging/ILogger.hpp"

#include "core/vasio/IVAsioPeer.hpp"
#include "core/internal/EndpointAddress.hpp"
#include "core/internal/MessageBuffer.hpp"
#include "core/vasio/RingBuffer.hpp"
#include "core/vasio/ReceiveBlobPool.hpp"
#include "core/vasio/VAsioPeerInfo.hpp"
#include "core/internal/ProtocolVersion.hpp"

#include "core/vasio/io/IIoContext.hpp"
#include "core/vasio/io/IRawByteStream.hpp"
#include "core/vasio/io/ITimer.hpp"

#include "core/vasio/PeerMetrics.hpp"

namespace SilKit {
namespace Core {

class VAsioPeer
    : public IVAsioPeer
    , private IRawByteStreamListener
    , private ITimerListener
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    VAsioPeer() = delete;
    VAsioPeer(const VAsioPeer& other) = delete;
    VAsioPeer(VAsioPeer&& other) = delete; //clang warning: implicitly deleted because of mutex

    VAsioPeer& operator=(const VAsioPeer& other) = delete;
    VAsioPeer& operator=(VAsioPeer&& other) = delete; //implicitly deleted because of mutex

    //! receiveBlobPool must outlive the peer and is only used on the io thread.
    VAsioPeer(IVAsioPeerListener* listener, IIoContext* ioContext, std::unique_ptr<IRawByteStream> stream,
              Services::Logging::ILoggerInternal* logger, std::unique_ptr<VSilKit::IPeerMetrics> metrics,
              ReceiveBlobPool* receiveBlobPool);

    ~VAsioPeer() override;

public:
    // ----------------------------------------
    // Public Methods
    void SendSilKitMsg(SerializedMessage buffer) override;
    void SendSilKitMsg(const SharedSerializedMessage& msg, EndpointId remoteIdx) override;
    void Subscribe(VAsioMsgSubscriber subscriber) override;

    auto GetInfo() const -> const VAsioPeerInfo& override;
    void SetInfo(VAsioPeerInfo info) override;
    //!< Return the socket address as URI encoded string or throw if not connected
    auto GetRemoteAddress() const -> std::string override;
    auto GetLocalAddress() const -> std::string override;

    void SetSimulationName(const std::string& simulationName) override;
    auto GetSimulationName() const -> const std::string& override;

    void StartAsyncRead() override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

    inline void SetProtocolVersion(ProtocolVersion v) override;
    inline auto GetProtocolVersion() const -> ProtocolVersion override;

    void Shutdown() override;

    void EnableAggregation() override;

    void InitializeMetrics(VSilKit::IMetricsManager* manager) override;

private:
    // ----------------------------------------
    // Private Methods
    //! \brief One queued write: inline bytes, followed by at most one of ownedBody or sharedBody.
    struct SendItem
    {
        static constexpr size_t MaxHeaderSize{32};

        //! Messages up to this size are inlined whole. Covers bus sized messages, CAN FD included.
        static constexpr size_t MaxInlineSize{128};

        //! Either the whole message, or just the network header when a body follows.
        std::array<uint8_t, MaxInlineSize> inlineData{};
        size_t inlineSize{0};

        std::vector<uint8_t> ownedBody;
        //! Shared with the other peers this message was sent to.
        SilKit::Util::SharedSpan<uint8_t> sharedBody;
    };

    void StartAsyncWrite();
    void WriteSomeAsync();
    void ReadSomeAsync();
    void DispatchBuffer();
    void EnqueueSendItem(SendItem item);
    static auto MakeSendItem(std::vector<uint8_t> blob) -> SendItem;
    void DispatchSendItem(SendItem item, MessageAggregationKind aggregationKind);
    void BuildCurrentSendingBuffers();
    void Aggregate(const SendItem& item);
    void Flush();

private: // IRawByteStreamListener
    void OnAsyncReadSomeDone(IRawByteStream& stream, size_t bytesTransferred) override;
    void OnAsyncWriteSomeDone(IRawByteStream& stream, size_t bytesTransferred) override;
    void OnShutdown(IRawByteStream& stream) override;

    // ITimerListener
    void OnTimerExpired(ITimer& timer) override;

private:
    // ----------------------------------------
    // Private Members
    ProtocolVersion _protocolVersion{};
    IVAsioPeerListener* _listener{nullptr};
    IIoContext* _ioContext{nullptr};
    std::unique_ptr<IRawByteStream> _socket;
    VAsioPeerInfo _info;
    std::string _simulationName;

    Services::Logging::ILoggerInternal* _logger;

    std::atomic_bool _isShuttingDown{false};

    // receiving
    std::atomic<uint32_t> _currentMsgSize{0u};
    RingBuffer _msgBuffer;
    std::vector<MutableBuffer> _currentReceivingBuffers;
    ReceiveBlobPool* _receiveBlobPool{nullptr};

    // sending
    mutable std::mutex _sendingQueueMutex;
    std::deque<SendItem> _sendingQueue;
    // NB: _currentSendingBuffers points into _currentSendItem, including its inline array, so move
    //     the item into place before building the buffers.
    SendItem _currentSendItem;
    std::vector<ConstBuffer> _currentSendingBuffers;
    std::vector<uint8_t> _aggregatedMessages;

    std::atomic_bool _sending{false};
    Core::ServiceDescriptor _serviceDescriptor;

    bool _useAggregation{false};
    const size_t _aggregationBufferThreshold{100 * 1000};

    // we trigger a flush of aggregated messages, if too much time has passed since the last flush
    std::unique_ptr<ITimer> _flushTimer;
    const std::chrono::milliseconds _flushTimeout{50};
    bool _initialTimerStarted{false};
    std::unique_ptr<VSilKit::IPeerMetrics> _peerMetrics;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void VAsioPeer::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}
auto VAsioPeer::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

void VAsioPeer::SetProtocolVersion(ProtocolVersion v)
{
    _protocolVersion = std::move(v);
}

auto VAsioPeer::GetProtocolVersion() const -> ProtocolVersion
{
    return _protocolVersion;
}


} // namespace Core
} // namespace SilKit
