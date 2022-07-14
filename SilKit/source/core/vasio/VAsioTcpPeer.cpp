// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "VAsioTcpPeer.hpp"

#include <iomanip>
#include <sstream>

#include "silkit/services/logging/ILogger.hpp"

#include "VAsioMsgKind.hpp"
#include "VAsioConnection.hpp"
#include "Uri.hpp"
#include "Assert.hpp"

using namespace asio::ip;

#ifdef __linux__

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

static void SetConnectOptions(SilKit::Services::Logging::ILogger* ,
    asio::generic::stream_protocol::socket&)
{
    // nothing specific required
}

static void EnableQuickAck(SilKit::Services::Logging::ILogger* log, asio::generic::stream_protocol::socket& socket)
{
    int val{1};
    //Disable Delayed Acknowledgments on the receiving side
    int e = setsockopt(socket.native_handle(), IPPROTO_TCP, TCP_QUICKACK,
        (void*)&val, sizeof(val));
    if (e != 0)
    {
        log->Warn("VasioTcpPeer: cannot set linux-specific socket option TCP_QUICKACK.");
    }
}

#else  // windows

#include <mstcpip.h>
#  if defined(__MINGW32__)

static void SetConnectOptions(SilKit::Services::Logging::ILogger* ,
    asio::generic::stream_protocol::socket&)
{
    // SIO_LOOPBACK_FAST_PATH not defined
}

#   else

static void SetConnectOptions(SilKit::Services::Logging::ILogger* logger,
    asio::generic::stream_protocol::socket& socket)
{
    // This should improve loopback performance, and have no effect on remote TCP/IP
    int enabled = 1;
    DWORD numberOfBytes = 0;
    auto result = WSAIoctl(socket.native_handle(),
        SIO_LOOPBACK_FAST_PATH,
        &enabled,
        sizeof(enabled),
        nullptr,
        0,
        &numberOfBytes,
        0,
        0);

    if (result == SOCKET_ERROR)
    {
        auto lastError = ::GetLastError();
        logger->Warn("VAsioTcpPeer: Setting Loopback FastPath failed: WSA IOCtl last error: {}", lastError);
    }
}

#   endif //!__MINGW32__

static void EnableQuickAck(SilKit::Services::Logging::ILogger* ,
    asio::generic::stream_protocol::socket& )

{
    //not supported
}

#endif // __linux__

static auto strip(std::string value, const std::string& chars) -> std::string
{
    size_t it;
    while((it = value.find_first_of(chars)) != value.npos)
    {
        value.erase(it, 1);
    }

    return value;
}

namespace SilKit {
namespace Core {

VAsioTcpPeer::VAsioTcpPeer(asio::any_io_executor executor, VAsioConnection* connection, Services::Logging::ILogger* logger)
    : _socket{executor}
    , _connection{connection}
    , _logger{logger}
{
}

bool VAsioTcpPeer::IsErrorToTryAgain(const asio::error_code& ec)
{
    return ec == asio::error::no_descriptors
        || ec == asio::error::no_buffer_space
        || ec == asio::error::no_memory
        || ec == asio::error::timed_out
        || ec == asio::error::try_again;
}

void VAsioTcpPeer::Shutdown()
{
    if (_socket.is_open())
    {
        _logger->Info("Shutting down connection to {}", _info.participantName);
        _connection->OnPeerShutdown(this);
        _socket.close();
    }
}


auto VAsioTcpPeer::GetInfo() const -> const VAsioPeerInfo&
{
    return _info;
}

void VAsioTcpPeer::SetInfo(VAsioPeerInfo peerInfo)
{
    _info = std::move(peerInfo);
}

static auto GetSocketAddress(const asio::generic::stream_protocol::socket& socket,
    bool remoteEndpoint) -> std::string 
{
    std::ostringstream out;
    const auto& epFamily = socket.local_endpoint().protocol().family();
    if (epFamily == asio::ip::tcp::v4().family()
        || epFamily == asio::ip::tcp::v6().family())
    {
        if (remoteEndpoint)
        {
            const auto& ep = socket.remote_endpoint();
            out << "tcp://" << reinterpret_cast<const asio::ip::tcp::endpoint&>(ep);
        }
        else
        {
            const auto& ep = socket.local_endpoint();
            out << "tcp://" << reinterpret_cast<const asio::ip::tcp::endpoint&>(ep);
        }
    }
    else if (epFamily == asio::local::stream_protocol{}.family())
    {
        // NB: remote and local endpoints are the same for local domain sockets.
        // The underlying sockaddr_un contains the path, zero terminated.
        const auto& ep = socket.local_endpoint();
        const auto* data = static_cast<const char*>(ep.data()->sa_data);
        out << "local://"  << data;
    }
    else
    {
        throw std::runtime_error("VAsioTcpPeer::GetSocketAddress(): Unknown endpoint.");
    }
    return out.str();
}


auto VAsioTcpPeer::GetRemoteAddress() const -> std::string
{
    return GetSocketAddress(_socket, true);
}

auto VAsioTcpPeer::GetLocalAddress() const -> std::string
{
    return GetSocketAddress(_socket, false);
}

bool VAsioTcpPeer::ConnectLocal(const std::string& socketPath)
{
    if (!_connection->Config().middleware.enableDomainSockets)
    {
        return false;
    }

    try
    {
        _logger->Debug("VAsioTcpPeer: Connecting to {}", socketPath);
        asio::local::stream_protocol::endpoint ep{socketPath};
        _socket.connect(ep);
        return true;
    }
    catch (const std::exception&)
    {
        // reset the socket
        _socket = decltype(_socket){_socket.get_executor()};
        // move on to TCP connections
    }
    return false;
}

bool VAsioTcpPeer::ConnectTcp(const std::string& host, uint16_t port)
{
    tcp::resolver resolver(_socket.get_executor());
    tcp::resolver::results_type resolverResults;
    auto strippedHost = strip(host, "[]"); //no ipv6 brackets
    try
    {
        resolverResults = resolver.resolve(strippedHost, std::to_string(static_cast<int>(port)));
    }
    catch (asio::system_error& err)
    {
        _logger->Warn("Unable to resolve hostname \"{}:{}\": {}", strippedHost, port, err.what());
        return false;
    }
    auto config = _connection->Config();
    for (auto&& resolverEntry : resolverResults)
    {
        try
        {
            _logger->Debug( "VAsioTcpPeer: Connecting to [{}]:{} ({})",
                resolverEntry.host_name(),
                resolverEntry.service_name(),
                (resolverEntry.endpoint().protocol().family() == asio::ip::tcp::v4().family() ? "TCPv4" : "TCPv6")
            );
            // Set  pre-connection platform options
            _socket.open(resolverEntry.endpoint().protocol());
            SetConnectOptions(_logger, _socket);

            _socket.connect(resolverEntry.endpoint());

            if (config.middleware.tcpNoDelay)
            {
                _socket.set_option(asio::ip::tcp::no_delay{true});
            }

            if (config.middleware.tcpQuickAck)
            {
                _enableQuickAck = true;
                EnableQuickAck(_logger, _socket);
            }

            if(config.middleware.tcpReceiveBufferSize > 0)
            {
                _socket.set_option(asio::socket_base::receive_buffer_size{config.middleware.tcpReceiveBufferSize});
            }

            if (config.middleware.tcpSendBufferSize > 0)
            {
                _socket.set_option(asio::socket_base::send_buffer_size{config.middleware.tcpSendBufferSize});
            }


            return true;
        }
        catch (asio::system_error& err)
        {
            // reset the socket
            _logger->Debug("VAsioTcpPeer: connect failed: {}", err.what());
            _socket = decltype(_socket){_socket.get_executor()};
        }
    }
    return false;
}
void VAsioTcpPeer::Connect(VAsioPeerInfo peerInfo)
{
    _info = std::move(peerInfo);

    std::stringstream attemptedUris;

    // parse endpoints into Uri objects
    const auto& uriStrings = _info.acceptorUris;
    std::vector<Uri> uris;
    std::transform(uriStrings.begin(), uriStrings.end(), std::back_inserter(uris),
        [](const auto& uriStr) {
            return Uri{uriStr};
    });

    if (_connection->Config().middleware.enableDomainSockets)
    {
        //Attempt local connections first
        auto localUri = std::find_if(uris.begin(), uris.end(),
            [](const auto& uri) {
                return uri.Type() == Uri::UriType::Local;
        });
        if (localUri != uris.end())
        {
            attemptedUris << localUri->EncodedString() << ",";
            if (ConnectLocal(localUri->Path()))
            {
                return;
            }
        }
    }

    //New style tcp:// URIs 
    for (const auto& uri : uris)
    {
        if (uri.Type() == Uri::UriType::Tcp)
        {
            try
            {
                attemptedUris << uri.EncodedString() << ",";
                if (ConnectTcp(uri.Host(), uri.Port()))
                {
                    return;
                }
            }
            catch (...)
            {
                // reset the socket
                _socket = decltype(_socket){_socket.get_executor()};
            }
        }
    }

    if (!_socket.is_open())
    {
        auto errorMsg = fmt::format("Failed to connect to host URIs: \"{}\"",
            attemptedUris.str());
        _logger->Debug(errorMsg);
        _logger->Debug("Tried the following URIs: {}", attemptedUris.str());

        throw std::runtime_error{errorMsg};
    }
}

void VAsioTcpPeer::SendSilKitMsg(SerializedMessage buffer)
{
    std::unique_lock<std::mutex> lock{ _sendingQueueLock };

    _sendingQueue.push(buffer.ReleaseStorage());

    lock.unlock();

    asio::dispatch(_socket.get_executor(), [this]() { StartAsyncWrite(); });
}

void VAsioTcpPeer::StartAsyncWrite()
{
    if (_sending)
        return;

    std::unique_lock<std::mutex> lock{ _sendingQueueLock };
    if (_sendingQueue.empty())
        return;

    _sending = true;

    _currentSendingBufferData = std::move(_sendingQueue.front());
    _sendingQueue.pop();
    lock.unlock();

    _currentSendingBuffer = asio::buffer(_currentSendingBufferData.data(), _currentSendingBufferData.size());
    WriteSomeAsync();
}

void VAsioTcpPeer::WriteSomeAsync()
{
    _socket.async_write_some(_currentSendingBuffer,
        [this](const asio::error_code& error, std::size_t bytesWritten) {
            if (error && !IsErrorToTryAgain(error))
            {
                Shutdown();
                _sending = false;
                return;
            }

            if (bytesWritten < _currentSendingBuffer.size())
            {
                _currentSendingBuffer += bytesWritten;
                WriteSomeAsync();
                return;
            }

            _sending = false;
            StartAsyncWrite();
        }
    );
}

void VAsioTcpPeer::Subscribe(VAsioMsgSubscriber subscriber)
{
    _logger->Debug("Announcing subscription for [{}] {}", subscriber.networkName, subscriber.msgTypeName);

    SendSilKitMsg(SerializedMessage{subscriber});
}

void VAsioTcpPeer::StartAsyncRead()
{
    _currentMsgSize = 0u;

    _msgBuffer.resize(4096);
    _wPos = {0u};

    ReadSomeAsync();
}

void VAsioTcpPeer::ReadSomeAsync()
{
    SILKIT_ASSERT(_msgBuffer.size() > 0);
    auto* wPtr = _msgBuffer.data() + _wPos;
    auto  size = _msgBuffer.size() - _wPos;

    _socket.async_read_some(asio::buffer(wPtr, size),
        [this](const asio::error_code& error, std::size_t bytesRead)
        {
            if (error && !IsErrorToTryAgain(error))
            {
                Shutdown();
                return;
            }

            if (_enableQuickAck)
            {
                // On Linux, the TCP_QUICKACK might be reset after a read/recvmsg syscall.
                EnableQuickAck(_logger, _socket);
            }
            _wPos += bytesRead;
            DispatchBuffer();
        }
    );
}

void VAsioTcpPeer::DispatchBuffer()
{
    if (_currentMsgSize == 0)
    {
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
        _logger->Error("Received invalid Message Size: {}", _currentMsgSize);
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
            throw std::runtime_error{ "VAsioTcpPeer: Received message is too small to contain message size header" };
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

} // namespace Core
} // namespace SilKit
