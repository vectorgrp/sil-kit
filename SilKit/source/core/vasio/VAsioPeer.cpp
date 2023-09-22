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


using namespace std::chrono_literals;


namespace SilKit {
namespace Core {

// Private constructor
VAsioPeer::VAsioPeer(IIoContext& ioContext, VAsioConnection* connection, Services::Logging::ILogger* logger)
    : _ioContext{&ioContext}
    , _connection{connection}
    , _logger{logger}
{
}

VAsioPeer::VAsioPeer(std::unique_ptr<IRawByteStream> stream, VAsioConnection* connection,
                     Services::Logging::ILogger* logger)
    : _ioContext{&stream->GetIoContext()}
    , _socket{std::move(stream)}
    , _connection{connection}
    , _logger{logger}
{
    _socket->SetListener(*this);
}

VAsioPeer::~VAsioPeer()
{
    SILKIT_TRACE_METHOD(_logger, "()");
}


void VAsioPeer::DrainAllBuffers()
{
    _isShuttingDown = true;

    Shutdown();
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

bool VAsioPeer::ConnectLocal(const std::string& socketPath)
{
    if (!_connection->Config().middleware.enableDomainSockets)
    {
        return false;
    }

    SilKit::Services::Logging::Debug(_logger, "ConnectLocal: Connecting to {}", socketPath);

    std::error_code errorCode;

    _socket = _ioContext->ConnectLocal(socketPath, errorCode);
    if (_socket == nullptr)
    {
        SilKit::Services::Logging::Debug(_logger, "ConnectLocal: Error while connecting to '{}': {}", socketPath,
                                         errorCode.message());
        return false;
    }

    _socket->SetListener(*this);

    return true;
}

bool VAsioPeer::ConnectTcp(const std::string& host, uint16_t port)
{
    auto resolverResults = _ioContext->Resolve(host);
    if (resolverResults.empty())
    {
        SilKit::Services::Logging::Warn(_logger, "Unable to resolve hostname \"{}:{}\"", host, port);
        return false;
    }

    for (const auto& address : resolverResults)
    {
        SilKit::Services::Logging::Debug(_logger, "ConnectTcp: Connecting to [{}]:{}", address, port);

        std::error_code errorCode;

        _socket = _ioContext->ConnectTcp(address, port, errorCode);
        if (_socket == nullptr)
        {
            SilKit::Services::Logging::Debug(_logger, "ConnectTcp: Error while connecting to '{}:{}': {}", host, port,
                                             errorCode.message());
            continue;
        }

        _socket->SetListener(*this);

        return true;
    }

    return false;
}

void VAsioPeer::Connect(VAsioPeerInfo peerInfo, std::stringstream& attemptedUris, bool& success)
{
    _info = std::move(peerInfo);

    // parse endpoints into Uri objects
    const auto& uriStrings = _info.acceptorUris;
    std::vector<Uri> uris;
    std::transform(uriStrings.begin(), uriStrings.end(), std::back_inserter(uris), [](const auto& uriStr) {
        return Uri::Parse(uriStr);
    });

    success = true;

    // Attempt connecting via local-domain socket first
    if (_connection->Config().middleware.enableDomainSockets)
    {
        for (const auto& uri : uris)
        {
            if (uri.Type() != Uri::UriType::Local)
            {
                continue;
            }

            attemptedUris << uri.EncodedString() << ",";

            if (ConnectLocal(uri.Path()))
            {
                return;
            }
        }
    }

    for (const auto& uri : uris)
    {
        if (uri.Type() != Uri::UriType::Tcp)
        {
            continue;
        }

        attemptedUris << uri.EncodedString() << ",";

        if (ConnectTcp(uri.Host(), uri.Port()))
        {
            return;
        }
    }

    if (_socket == nullptr)
    {
        SilKit::Services::Logging::Debug(_logger, "Tried the following URIs: {}", attemptedUris.str());
    }

    success = false;
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
        _connection->OnSocketData(this, std::move(message));

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
    SILKIT_TRACE_METHOD(_logger, "({}, {})", static_cast<const void*>(&stream), bytesTransferred);

    _wPos += bytesTransferred;
    DispatchBuffer();
}


void VAsioPeer::OnAsyncWriteSomeDone(IRawByteStream& stream, size_t bytesTransferred)
{
    SILKIT_UNUSED_ARG(stream);
    SILKIT_TRACE_METHOD(_logger, "({}, {})", static_cast<const void*>(&stream), bytesTransferred);

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
    SILKIT_TRACE_METHOD(_logger, "({})", static_cast<const void*>(&stream));

    _connection->OnPeerShutdown(this);
}


} // namespace Core
} // namespace SilKit
