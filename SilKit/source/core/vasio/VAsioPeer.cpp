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

#include "ILogger.hpp"
#include "VAsioMsgKind.hpp"
#include "VAsioConnection.hpp"
#include "Uri.hpp"
#include "Assert.hpp"

#include "util/TracingMacros.hpp"


#if SILKIT_ENABLE_TRACING_INSTRUMENTATION_VAsioPeer
#    define SILKIT_TRACE_METHOD_(logger, ...) SILKIT_TRACE_METHOD(logger, __VA_ARGS__)
#else
#    define SILKIT_TRACE_METHOD_(...)
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
{
    _socket->SetListener(*this);
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
    // Prevent sending when shutting down
    if (!_isShuttingDown && _socket != nullptr)
    {
        std::unique_lock<std::mutex> lock{_sendingQueueMutex};

        _sendingQueue.push_back(buffer.ReleaseStorage());

        lock.unlock();

        _ioContext->Dispatch([this] {
            StartAsyncWrite();
        });
    }
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
    Services::Logging::Debug(_logger, "VAsioTcpPeer: Subscribing to messages of type '{}' on link '{}' from participant '{}'",
                             subscriber.msgTypeName, subscriber.networkName, _info.participantName);
    SendSilKitMsg(SerializedMessage{subscriber});
}

void VAsioPeer::StartAsyncRead()
{
    _currentMsgSize = 0u;

    _msgBuffer.resize(4096);
    _wPos = {0u};

    ReadSomeAsync();
}

void VAsioPeer::ReadSomeAsync()
{
    SILKIT_ASSERT(_msgBuffer.size() > 0);
    auto* wPtr = _msgBuffer.data() + _wPos;
    auto  size = _msgBuffer.size() - _wPos;

    _currentReceivingBuffer = MutableBuffer{wPtr, size};

    _socket->AsyncReadSome(MutableBufferSequence{&_currentReceivingBuffer, 1});
}

void VAsioPeer::DispatchBuffer()
{
    if (_currentMsgSize == 0)
    {
        if (_isShuttingDown)
        {
            return;
        }
        if (_wPos >= sizeof(uint32_t))
        {
            _currentMsgSize = *reinterpret_cast<uint32_t*>(_msgBuffer.data());
        }
        else
        {
            // not enough data to even determine the message size...
            // make sure the buffer can store some data
            if (_msgBuffer.size() < sizeof(uint32_t))
                _msgBuffer.resize(4096);
            // and restart the async read operation
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


    if (_wPos < _currentMsgSize)
    {
        // Make the buffer large enough and wait until we have more data.
        _msgBuffer.resize(_currentMsgSize);
        ReadSomeAsync();
        return;
    }
    else
    {
        auto newBuffer = std::vector<uint8_t>{_msgBuffer.begin() + _currentMsgSize, _msgBuffer.end()};
        auto newWPos = _wPos - _currentMsgSize;

        // manually extract the message size so we can adjust the MessageBuffer size
        uint32_t msgSize{0u};
        if (_msgBuffer.size() < sizeof msgSize)
        {
            throw SilKitError{ "DispatchBuffer: Received message is too small to contain message size header" };
        }
        memcpy(&msgSize, _msgBuffer.data(), sizeof msgSize);
        //ensure buffer does not contain data from contiguous messages
        _msgBuffer.resize(msgSize);
        SerializedMessage message{std::move(_msgBuffer)};
        message.SetProtocolVersion(GetProtocolVersion());
        _listener->OnSocketData(this, std::move(message));

        // keep trailing data in the buffer
        _msgBuffer = std::move(newBuffer);
        _wPos = newWPos;
        _currentMsgSize = 0u;

        DispatchBuffer();
    }
}


// IRawByteStreamListener


void VAsioPeer::OnAsyncReadSomeDone(IRawByteStream& stream, size_t bytesTransferred)
{
    SILKIT_UNUSED_ARG(stream);
    SILKIT_TRACE_METHOD_(_logger, "({}, {})", static_cast<const void*>(&stream), bytesTransferred);

    _wPos += bytesTransferred;
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


} // namespace Core
} // namespace SilKit


#undef SILKIT_TRACE_METHOD_
