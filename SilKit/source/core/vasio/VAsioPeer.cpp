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

#include "VAsioPeer.hpp"

#include <iomanip>
#include <sstream>
#include <thread>

#include "ILoggerInternal.hpp"
#include "VAsioMsgKind.hpp"
#include "VAsioConnection.hpp"
#include "Uri.hpp"
#include "Assert.hpp"

#include "util/TracingMacros.hpp"


#if SILKIT_ENABLE_TRACING_INSTRUMENTATION_VAsioPeer
#define SILKIT_TRACE_METHOD_(logger, ...) SILKIT_TRACE_METHOD(logger, __VA_ARGS__)
#else
#define SILKIT_TRACE_METHOD_(...)
#endif


using namespace std::chrono_literals;


namespace SilKit {
namespace Core {

VAsioPeer::VAsioPeer(IVAsioPeerListener* listener, IIoContext* ioContext, std::unique_ptr<IRawByteStream> stream,
                     Services::Logging::ILogger* logger)
    : _listener{listener}
    , _ioContext{ioContext}
    , _socket{std::move(stream)}
    , _logger{logger}
    , _msgBuffer{4096}
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
    auto blob = buffer.ReleaseStorage();

    if (_useAggregation && buffer.GetAggregationKind() == MessageAggregationKind::UserDataMessage)
    {
        Aggregate(blob);
    }
    else if (_useAggregation && buffer.GetAggregationKind() == MessageAggregationKind::FlushAggregationMessage)
    {
        Aggregate(blob); // don't forget to send (current) time sync message
        Flush();
    }
    else
    {
        SendSilKitMsgInternal(std::move(blob));
    }
}

void VAsioPeer::SendSilKitMsgInternal(std::vector<uint8_t> blob)
{
    // Prevent sending when shutting down
    if (!_isShuttingDown && _socket != nullptr)
    {
        std::unique_lock<std::mutex> lock{_sendingQueueMutex};

        _sendingQueue.emplace_back(std::move(blob));

        lock.unlock();

        _ioContext->Dispatch([this] { StartAsyncWrite(); });
    }
}

void VAsioPeer::Aggregate(const std::vector<uint8_t>& blob)
{
    // start initial timer
    // NB: resetting timer in every Aggregate() is costly
    if (!_initialTimerStarted)
    {
        _flushTimer->AsyncWaitFor(_flushTimeout);
        _initialTimerStarted = true;
    }

    _aggregatedMessages.insert(_aggregatedMessages.end(), blob.begin(), blob.end());

    // ensure that the aggregation buffer does not exceed a certain size
    if (_aggregatedMessages.size() > _aggregationBufferThreshold)
    {
        Services::Logging::Debug(_logger,
                                 "VAsioPeer: Automated flush of aggregation buffer has been triggered, since the "
                                 "maximum buffer size of {}Byte has been exceeded.",
                                 _aggregationBufferThreshold);
        Flush();
    }
}

void VAsioPeer::Flush()
{
    decltype(_aggregatedMessages) blob;
    blob.swap(_aggregatedMessages);
    SendSilKitMsgInternal(std::move(blob));

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

    _currentSendingBufferData = std::move(_sendingQueue.front());
    _sendingQueue.pop_front();
    lock.unlock();

    _currentSendingBuffer = ConstBuffer(_currentSendingBufferData.data(), _currentSendingBufferData.size());
    WriteSomeAsync();
}

void VAsioPeer::WriteSomeAsync()
{
    _socket->AsyncWriteSome(ConstBufferSequence{&_currentSendingBuffer, 1});
}

void VAsioPeer::Subscribe(VAsioMsgSubscriber subscriber)
{
    Services::Logging::Debug(_logger,
                             "VAsioTcpPeer: Subscribing to messages of type '{}' on link '{}' from participant '{}'",
                             subscriber.msgTypeName, subscriber.networkName, _info.participantName);
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
    if (_currentMsgSize == 0)
    {
        if (_isShuttingDown)
        {
            return;
        }
        if (_msgBuffer.Size() >= sizeof(uint32_t))
        {
            std::vector<uint8_t> msgSizeInBytes(sizeof(uint32_t));
            if (!_msgBuffer.Peek(msgSizeInBytes))
            {
                throw SilKitError("Reading message size from ring buffer failed.");
            }
            _currentMsgSize = *reinterpret_cast<uint32_t*>(msgSizeInBytes.data());
        }
        else
        {
            // not enough data to even determine the message size...
            // restart the async read operation
            ReadSomeAsync();
            return;
        }
    }

    // validate the received size
    if (_currentMsgSize == 0 || _currentMsgSize > 1024 * 1024 * 1024)
    {
        SilKit::Services::Logging::Error(_logger, "Received invalid Message Size: {}", _currentMsgSize);
        Shutdown();
    }


    if (_msgBuffer.Size() < _currentMsgSize)
    {
        // Make the buffer large enough and wait until we have more data.
        if (_msgBuffer.Capacity() < _currentMsgSize)
        {
            _msgBuffer.Reserve(_currentMsgSize);
        }

        ReadSomeAsync();
        return;
    }
    else
    {
        std::vector<uint8_t> currentMsg(_currentMsgSize);
        if (!_msgBuffer.Read(currentMsg))
        {
            throw SilKitError("Reading data from ring buffer failed.");
        }

        SerializedMessage message{std::move(currentMsg)};
        message.SetProtocolVersion(GetProtocolVersion());
        _listener->OnSocketData(this, std::move(message));

        _currentMsgSize = 0u;

        DispatchBuffer();
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

    if (bytesTransferred < _currentSendingBuffer.GetSize())
    {
        _currentSendingBuffer.SliceOff(bytesTransferred);
        WriteSomeAsync();
        return;
    }

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
        Services::Logging::Warn(
            _logger,
            "VAsioPeer: Automated flush of aggregation buffer has been triggered, since the "
            "maximum allowed time step duration of {}milliseconds has been exceeded. Consider switching off the "
            "message aggregation via the config option 'EnableMessageAggregation'.",
            _flushTimeout.count());

        Flush();
    }
}

void VAsioPeer::EnableAggregation()
{
    _useAggregation = true;
    SilKit::Services::Logging::Debug(_logger, "VAsioPeer: Enable aggregation for peer {}", _info.participantName);
}

} // namespace Core
} // namespace SilKit


#undef SILKIT_TRACE_METHOD_
