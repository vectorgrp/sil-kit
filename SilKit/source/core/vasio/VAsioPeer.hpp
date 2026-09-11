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

    VAsioPeer(IVAsioPeerListener* listener, IIoContext* ioContext, std::unique_ptr<IRawByteStream> stream,
              Services::Logging::ILoggerInternal* logger, std::unique_ptr<VSilKit::IPeerMetrics> metrics);

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
    /*! \brief One queued write: an optional per-peer header followed by a shared body.
     *
     * A message that is sent to several peers differs only in the remote index inside its network
     * header, so the body can be shared between peers and only the small header is per-peer. The
     * header is stored inline; headerSize == 0 marks an item that consists of the body alone.
     */
    struct SendItem
    {
        static constexpr size_t kMaxHeaderSize = 32;

        std::array<uint8_t, kMaxHeaderSize> header{};
        size_t headerSize{0};
        SilKit::Util::SharedSpan<uint8_t> body;
    };

    void StartAsyncWrite();
    void WriteSomeAsync();
    void ReadSomeAsync();
    void DispatchBuffer();
    void SendSilKitMsgInternal(std::vector<uint8_t> blob);
    void EnqueueSendItem(SendItem item);
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

    // sending
    mutable std::mutex _sendingQueueMutex;
    std::deque<SendItem> _sendingQueue;
    // NB: _currentSendingBuffers points into _currentSendItem, including into its inline header
    //     array, so the item must be moved into place before the buffers are built.
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
