// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include <vector>
#include <queue>
#include <mutex>
#include <sstream>

#include "silkit/services/logging/ILogger.hpp"

#include "IVAsioPeer.hpp"
#include "EndpointAddress.hpp"
#include "MessageBuffer.hpp"
#include "RingBuffer.hpp"
#include "VAsioPeerInfo.hpp"
#include "ProtocolVersion.hpp"

#include "IIoContext.hpp"
#include "IRawByteStream.hpp"
#include "ITimer.hpp"

#include "PeerMetrics.hpp"

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
              Services::Logging::ILogger* logger, std::unique_ptr<VSilKit::IPeerMetrics> metrics);

    ~VAsioPeer() override;

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
    void StartAsyncWrite();
    void WriteSomeAsync();
    void ReadSomeAsync();
    void DispatchBuffer();
    void SendSilKitMsgInternal(std::vector<uint8_t> blob);
    void Aggregate(const std::vector<uint8_t>& blob);
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

    Services::Logging::ILogger* _logger;

    std::atomic_bool _isShuttingDown{false};

    // receiving
    std::atomic<uint32_t> _currentMsgSize{0u};
    RingBuffer _msgBuffer;
    std::vector<MutableBuffer> _currentReceivingBuffers;

    // sending
    mutable std::mutex _sendingQueueMutex;
    std::deque<std::vector<uint8_t>> _sendingQueue;
    ConstBuffer _currentSendingBuffer;
    std::vector<uint8_t> _currentSendingBufferData;
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
