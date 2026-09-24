// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/vasio/VAsioPeer.hpp"

#include <array>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <thread>

#include "services/logging/LoggerMessage.hpp"
#include "core/vasio/VAsioMsgKind.hpp"
#include "core/vasio/VAsioConnection.hpp"
#include "util/Uri.hpp"
#include "util/Assert.hpp"

#include "core/vasio/io/util/TracingMacros.hpp"


#if SILKIT_ENABLE_TRACING_INSTRUMENTATION_VAsioPeer
#define SILKIT_TRACE_METHOD_(logger, ...) SILKIT_TRACE_METHOD(logger, __VA_ARGS__)
#else
#define SILKIT_TRACE_METHOD_(...)
#endif


using namespace std::chrono_literals;


namespace SilKit {
namespace Core {

VAsioPeer::VAsioPeer(IVAsioPeerListener* listener, IIoContext* ioContext, std::unique_ptr<IRawByteStream> stream,
                     Services::Logging::ILoggerInternal* logger, std::unique_ptr<VSilKit::IPeerMetrics> peerMetrics,
                     ReceiveBlobPool* receiveBlobPool)
    : _listener{listener}
    , _ioContext{ioContext}
    , _socket{std::move(stream)}
    , _logger{logger}
    , _msgBuffer{4096}
    , _receiveBlobPool{receiveBlobPool}
    , _peerMetrics{std::move(peerMetrics)}
{
    _socket->SetListener(*this);

    // set up timer (guarantees working communication in case of message aggregation)
    _flushTimer = _ioContext->MakeTimer();
    _flushTimer->SetListener(*this);
}

VAsioPeer::~VAsioPeer()
{
    SILKIT_TRACE_METHOD_(_logger, "()");
}


void VAsioPeer::Shutdown()
{
    _isShuttingDown = true;

    {
        std::unique_lock<decltype(_sendingQueueMutex)> lock{_sendingQueueMutex};
        _sendingQueue.clear();
    }

    _socket->Shutdown();
    _flushTimer->Shutdown();
}


auto VAsioPeer::GetInfo() const -> const VAsioPeerInfo&
{
    return _info;
}

void VAsioPeer::SetInfo(VAsioPeerInfo peerInfo)
{
    _info = std::move(peerInfo);
}


auto VAsioPeer::GetRemoteAddress() const -> std::string
{
    return _socket->GetRemoteEndpoint();
}

auto VAsioPeer::GetLocalAddress() const -> std::string
{
    return _socket->GetLocalEndpoint();
}

void VAsioPeer::SetSimulationName(const std::string& simulationName)
{
    _simulationName = simulationName;
    _serviceDescriptor.SetSimulationName(simulationName);
}

auto VAsioPeer::GetSimulationName() const -> const std::string&
{
    return _simulationName;
}

void VAsioPeer::SendSilKitMsg(SerializedMessage buffer)
{
    const auto aggregationKind = buffer.GetAggregationKind();

    _peerMetrics->TxBytes(buffer.GetStorageSize());
    _peerMetrics->TxPacket();

    DispatchSendItem(MakeSendItem(buffer.ReleaseStorage()), aggregationKind);
}

void VAsioPeer::SendSilKitMsg(const SharedSerializedMessage& msg, EndpointId remoteIdx)
{
    _peerMetrics->TxBytes(msg.TotalSize());
    _peerMetrics->TxPacket();

    SILKIT_ASSERT(msg.HeaderSize() <= SendItem::MaxHeaderSize);
    SILKIT_ASSERT(msg.RemoteIndexOffset() + sizeof(EndpointId) <= msg.HeaderSize());

    SendItem item;

    if (msg.TotalSize() <= SendItem::MaxInlineSize)
    {
        // NB: one memcpy is cheaper than the reference count traffic and the split write.
        item.inlineSize = msg.TotalSize();
        std::memcpy(item.inlineData.data(), msg.Blob().data(), item.inlineSize);
    }
    else
    {
        item.inlineSize = msg.HeaderSize();
        std::memcpy(item.inlineData.data(), msg.Header().data(), item.inlineSize);
        item.sharedBody = msg.Body();
    }

    // NB: same encoding as MessageBuffer uses for an EndpointId.
    std::memcpy(item.inlineData.data() + msg.RemoteIndexOffset(), &remoteIdx, sizeof(remoteIdx));

    DispatchSendItem(std::move(item), msg.GetAggregationKind());
}

void VAsioPeer::DispatchSendItem(SendItem item, MessageAggregationKind aggregationKind)
{
    if (_useAggregation && aggregationKind == MessageAggregationKind::UserDataMessage)
    {
        Aggregate(item);
    }
    else if (_useAggregation && aggregationKind == MessageAggregationKind::FlushAggregationMessage)
    {
        Aggregate(item); // don't forget to send (current) time sync message
        Flush();
    }
    else
    {
        EnqueueSendItem(std::move(item));
    }
}

auto VAsioPeer::MakeSendItem(std::vector<uint8_t> blob) -> SendItem
{
    SendItem item;

    if (blob.size() <= SendItem::MaxInlineSize)
    {
        item.inlineSize = blob.size();
        std::memcpy(item.inlineData.data(), blob.data(), item.inlineSize);
        // NB: the bytes live in the item now, so hand the buffer back for the next serialization
        //     instead of letting it be freed.
        RecycleSerializationBuffer(std::move(blob));
    }
    else
    {
        // NB: nothing shares these bytes, so a SharedSpan would only add an allocation.
        item.ownedBody = std::move(blob);
    }

    return item;
}

void VAsioPeer::EnqueueSendItem(SendItem item)
{
    // Prevent sending when shutting down
    if (!_isShuttingDown && _socket != nullptr)
    {
        std::unique_lock<std::mutex> lock{_sendingQueueMutex};

        _sendingQueue.emplace_back(std::move(item));

        _peerMetrics->TxQueueSize(_sendingQueue.size());

        lock.unlock();

        _ioContext->Dispatch([this] { StartAsyncWrite(); });
    }
}

void VAsioPeer::BuildCurrentSendingBuffers()
{
    _currentSendingBuffers.clear();

    if (_currentSendItem.inlineSize > 0)
    {
        _currentSendingBuffers.emplace_back(_currentSendItem.inlineData.data(), _currentSendItem.inlineSize);
    }

    if (!_currentSendItem.ownedBody.empty())
    {
        _currentSendingBuffers.emplace_back(_currentSendItem.ownedBody.data(),
                                            _currentSendItem.ownedBody.size());
    }
    else
    {
        const auto body = _currentSendItem.sharedBody.AsSpan();
        if (!body.empty())
        {
            _currentSendingBuffers.emplace_back(body.data(), body.size());
        }
    }
}

void VAsioPeer::Aggregate(const SendItem& item)
{
    // start initial timer
    // NB: resetting timer in every Aggregate() is costly
    if (!_initialTimerStarted)
    {
        _flushTimer->AsyncWaitFor(_flushTimeout);
        _initialTimerStarted = true;
    }

    if (item.inlineSize > 0)
    {
        _aggregatedMessages.insert(_aggregatedMessages.end(), item.inlineData.begin(),
                                   item.inlineData.begin() + static_cast<std::ptrdiff_t>(item.inlineSize));
    }

    if (!item.ownedBody.empty())
    {
        _aggregatedMessages.insert(_aggregatedMessages.end(), item.ownedBody.begin(), item.ownedBody.end());
    }
    else
    {
        const auto body = item.sharedBody.AsSpan();
        _aggregatedMessages.insert(_aggregatedMessages.end(), body.begin(), body.end());
    }

    // ensure that the aggregation buffer does not exceed a certain size
    if (_aggregatedMessages.size() > _aggregationBufferThreshold)
    {
        _logger->MakeMessage(Services::Logging::Level::Debug, TopicOf(*this))
            .SetMessage("VAsioPeer: Automated flush of aggregation buffer has been triggered, since the "
                        "maximum buffer size of {}Byte has been exceeded.",
                        _aggregationBufferThreshold)
            .Dispatch();
        Flush();
    }
}

void VAsioPeer::Flush()
{
    decltype(_aggregatedMessages) blob;
    blob.swap(_aggregatedMessages);
    EnqueueSendItem(MakeSendItem(std::move(blob)));

    // reset timer when flush is triggered
    _flushTimer->AsyncWaitFor(_flushTimeout);
}

void VAsioPeer::StartAsyncWrite()
{
    if (_sending)
        return;

    std::unique_lock<std::mutex> lock{_sendingQueueMutex};
    if (_sendingQueue.empty())
    {
        return;
    }

    _sending = true;

    _currentSendItem = std::move(_sendingQueue.front());
    _sendingQueue.pop_front();
    lock.unlock();

    BuildCurrentSendingBuffers();
    WriteSomeAsync();
}

void VAsioPeer::WriteSomeAsync()
{
    _socket->AsyncWriteSome(ConstBufferSequence{_currentSendingBuffers.data(), _currentSendingBuffers.size()});
}

void VAsioPeer::Subscribe(VAsioMsgSubscriber subscriber)
{
    _logger->MakeMessage(Services::Logging::Level::Debug, TopicOf(*this))
        .SetMessage("VAsioTcpPeer: Subscribing to messages of type '{}' on link '{}' from participant '{}'",
                    subscriber.msgTypeName, subscriber.networkName, _info.participantName)
        .Dispatch();
    SendSilKitMsg(SerializedMessage{subscriber});
}

void VAsioPeer::StartAsyncRead()
{
    _currentMsgSize = 0u;

    ReadSomeAsync();
}

void VAsioPeer::ReadSomeAsync()
{
    SILKIT_ASSERT(_msgBuffer.Capacity() - _msgBuffer.Size() > 0);

    _currentReceivingBuffers.clear();
    _msgBuffer.GetWritingBuffers(_currentReceivingBuffers);

    _socket->AsyncReadSome(MutableBufferSequence{_currentReceivingBuffers.data(), _currentReceivingBuffers.size()});
}

void VAsioPeer::DispatchBuffer()
{
    while(true) 
    {
        if (_currentMsgSize == 0)
        {
            if (_isShuttingDown)
            {
                break;
            }
            if (_msgBuffer.Size() >= sizeof(uint32_t))
            {
                std::array<uint8_t, sizeof(uint32_t)> msgSizeInBytes{};
                if (!_msgBuffer.Peek(SilKit::Util::MakeSpan(msgSizeInBytes)))
                {
                    throw SilKitError("Reading message size from ring buffer failed.");
                }
                uint32_t msgSize{0};
                std::memcpy(&msgSize, msgSizeInBytes.data(), sizeof(msgSize));
                _currentMsgSize = msgSize;
            }
            else
            {
                // not enough data to even determine the message size...
                // restart the async read operation
                ReadSomeAsync();
                break;
            }
        }

        // validate the received size
        if (_currentMsgSize == 0 || _currentMsgSize > 1024 * 1024 * 1024)
        {
            _logger->MakeMessage(Services::Logging::Level::Error, TopicOf(*this))
                .SetMessage("Received invalid Message Size: {}", _currentMsgSize.load())
                .Dispatch();
            Shutdown();
            break;
        }


        if (_msgBuffer.Size() < _currentMsgSize)
        {
            // Make the buffer large enough and wait until we have more data.
            if (_msgBuffer.Capacity() < _currentMsgSize)
            {
                _msgBuffer.Reserve(_currentMsgSize);
            }

            ReadSomeAsync();
            break;
        }
        else
        {
            // NB: the message must be linearised out of the ring buffer because it may wrap, but
            //     it is allocated as a shared blob so that deserialized payloads can alias it
            //     instead of being copied out again. The blob is filled before being wrapped,
            //     which establishes the immutability the SharedSpan invariant requires. One blob
            //     per message keeps the retained memory bounded by the message's own size.
            const size_t blobSize = _currentMsgSize;
            auto currentMsg = _receiveBlobPool->Acquire(blobSize);

            // NB: a pooled blob may be larger than this message, so read and view exactly the
            //     message's bytes rather than the whole buffer.
            if (!_msgBuffer.Read(SilKit::Util::Span<uint8_t>{currentMsg->data(), blobSize}))
            {
                throw SilKitError("Reading data from ring buffer failed.");
            }
            SerializedMessage message{
                SilKit::Util::MakeSharedSpan(std::shared_ptr<const std::vector<uint8_t>>{std::move(currentMsg)}, 0,
                                             blobSize)};
            message.SetProtocolVersion(GetProtocolVersion());

            _peerMetrics->RxBytes(message.GetStorageSize());
            _peerMetrics->RxPacket();

            _listener->OnSocketData(this, std::move(message));

            _currentMsgSize = 0u;
        }
    }
}


// IRawByteStreamListener


void VAsioPeer::OnAsyncReadSomeDone(IRawByteStream& stream, size_t bytesTransferred)
{
    SILKIT_UNUSED_ARG(stream);
    SILKIT_TRACE_METHOD_(_logger, "({}, {})", static_cast<const void*>(&stream), bytesTransferred);

    _msgBuffer.AdvanceWPos(bytesTransferred);
    DispatchBuffer();
}


void VAsioPeer::OnAsyncWriteSomeDone(IRawByteStream& stream, size_t bytesTransferred)
{
    SILKIT_UNUSED_ARG(stream);
    SILKIT_TRACE_METHOD_(_logger, "({}, {})", static_cast<const void*>(&stream), bytesTransferred);

    // Consume the transferred bytes across the buffer sequence: drop the buffers that were
    // written in full and slice the prefix off a partially written one.
    size_t remaining = bytesTransferred;
    auto it = _currentSendingBuffers.begin();
    while (it != _currentSendingBuffers.end() && remaining > 0)
    {
        if (remaining >= it->GetSize())
        {
            remaining -= it->GetSize();
            ++it;
        }
        else
        {
            it->SliceOff(remaining);
            remaining = 0;
        }
    }
    _currentSendingBuffers.erase(_currentSendingBuffers.begin(), it);

    if (!_currentSendingBuffers.empty())
    {
        WriteSomeAsync();
        return;
    }

    // release the body as soon as it has been written
    _currentSendItem = SendItem{};
    _sending = false;
    StartAsyncWrite();
}


void VAsioPeer::OnShutdown(IRawByteStream& stream)
{
    SILKIT_UNUSED_ARG(stream);
    SILKIT_TRACE_METHOD_(_logger, "({})", static_cast<const void*>(&stream));

    _listener->OnPeerShutdown(this);
}

void VAsioPeer::OnTimerExpired(ITimer& timer)
{
    SILKIT_UNUSED_ARG(timer);
    SILKIT_TRACE_METHOD_(_logger, "({})", static_cast<const void*>(&timer));

    if (!_aggregatedMessages.empty())
    {
        _logger->MakeMessage(Services::Logging::Level::Warn, TopicOf(*this))
            .SetMessage("VAsioPeer: Automated flush of aggregation buffer has been triggered, since the "
                "maximum allowed time step duration of {}milliseconds has been exceeded. Consider switching off the "
                "message aggregation via the config option 'EnableMessageAggregation'.",
                _flushTimeout.count())
            .Dispatch();
        Flush();
    }
}

void VAsioPeer::EnableAggregation()
{
    _useAggregation = true;

    _logger->MakeMessage(Services::Logging::Level::Debug, TopicOf(*this))
        .SetMessage("VAsioPeer: Enable aggregation for peer {}", _info.participantName)
        .Dispatch();
}

void VAsioPeer::InitializeMetrics(VSilKit::IMetricsManager* manager)
{
    _peerMetrics->InitializeMetrics(manager, this);
}

} // namespace Core
} // namespace SilKit


#undef SILKIT_TRACE_METHOD_
