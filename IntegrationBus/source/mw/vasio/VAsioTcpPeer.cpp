// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioTcpPeer.hpp"
#include "VAsioMsgKind.hpp"
#include "VAsioConnection.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

using namespace asio::ip;

#ifdef __linux__

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

static void EnableQuickAck(ib::mw::logging::ILogger* log, asio::generic::stream_protocol::socket& socket)
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

#else  //windows 

static void EnableQuickAck(ib::mw::logging::ILogger*, asio::generic::stream_protocol::socket& )
{
}

#endif  // __linux__

namespace ib {
namespace mw {

VAsioTcpPeer::VAsioTcpPeer(asio::any_io_executor executor, VAsioConnection* ibConnection, logging::ILogger* logger)
    : _socket{executor}
    , _ibConnection{ibConnection}
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
        _ibConnection->OnPeerShutdown(this);
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

void VAsioTcpPeer::ConnectLocal(const std::string& socketPath)
{
    try
    {
        asio::local::stream_protocol::endpoint ep{socketPath};
        _socket.connect(ep);
        return;
    }
    catch (...)
    {
        // reset the socket
        _socket = decltype(_socket){_socket.get_executor()};
        // move on to TCP connections
    }
}

void VAsioTcpPeer::ConnectTcp(const std::string& host, const std::string& port)
{
    tcp::resolver resolver(_socket.get_executor());
    tcp::resolver::results_type resolverResults;
    try
    {
        resolverResults = resolver.resolve(host, port);
    }
    catch (asio::system_error& err)
    {
        _logger->Warn("Unable to resolve hostname \"{}:{}\": {}", host, port, err.what());
        return;
    }
    const auto& config = _ibConnection->Config().middlewareConfig.vasio;
    for (auto&& resolverEntry : resolverResults)
    {
        try
        {
            _socket.connect(resolverEntry.endpoint());

            if (config.tcpNoDelay)
            {
                _socket.set_option(asio::ip::tcp::no_delay{true});
            }

            if (config.tcpQuickAck)
            {
                _enableQuickAck = true;
                EnableQuickAck(_logger, _socket);
            }

            if(config.tcpReceiveBufferSize > 0)
            {
                _socket.set_option(asio::socket_base::receive_buffer_size{config.tcpReceiveBufferSize});
            }

            if (config.tcpSendBufferSize > 0)
            {
                _socket.set_option(asio::socket_base::send_buffer_size{config.tcpSendBufferSize});
            }

            return;
        }
        catch (asio::system_error& /*err*/)
        {
            // reset the socket
            _socket = decltype(_socket){_socket.get_executor()};
        }
    }
}
void VAsioTcpPeer::Connect(VAsioPeerInfo peerInfo)
{
    SetInfo(std::move(peerInfo));

    std::stringstream attemptedUris;
    // Unix Domain Sockets: attempt to connect via local socket first
    const std::string local_prefix{"local://"};
    const auto& uris = GetInfo().acceptorUris;
    auto localUri = std::find_if(uris.begin(), uris.end(),
        [&local_prefix](const auto& uri) {
        return uri.find(local_prefix) == 0;
    });
    if (localUri != uris.end())
    {
        const auto filePath = localUri->substr(local_prefix.size());
        attemptedUris << filePath << ", ";
        ConnectLocal(filePath);
        if (_socket.is_open())
        {
            return;
        }
    }

    //New style tcp:// URIs 
    for (const auto& uri : GetInfo().acceptorUris)
    {
        const std::string prefix{"tcp://"};
        if (uri.find(prefix) == 0)
        {
            const auto hostAndPort = uri.substr(prefix.size());
            try
            {
                auto sep = hostAndPort.rfind(":");
                if (sep == hostAndPort.npos)
                {
                    continue;
                }
                const auto host = hostAndPort.substr(0, sep);
                const auto port = hostAndPort.substr(sep+1);
                attemptedUris << hostAndPort << ", ";
                ConnectTcp(host, port);
                if (_socket.is_open())
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

    // TCP: Fall back to TCP and resolve the host name using legacy members
    ConnectTcp(GetInfo().acceptorHost, std::to_string(GetInfo().acceptorPort));
    if (!_socket.is_open())
    {
        auto errorMsg = fmt::format("Failed to connect to host {}", GetInfo().acceptorHost);
        _logger->Error(errorMsg);
        _logger->Info("Tried the following URIs: {}", attemptedUris.str());

        throw std::runtime_error{errorMsg};
    }
}

void VAsioTcpPeer::SendIbMsg(MessageBuffer buffer)
{
    auto sendBuffer = buffer.ReleaseStorage();
    if (sendBuffer.size() > std::numeric_limits<uint32_t>::max())
        throw std::runtime_error{"Message is too large"};

    *reinterpret_cast<uint32_t*>(sendBuffer.data()) = static_cast<uint32_t>(sendBuffer.size());

    std::unique_lock<std::mutex> lock{ _sendingQueueLock };

    _sendingQueue.push(std::move(sendBuffer));

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
    MessageBuffer buffer;
    uint32_t rawMsgSize{0};
    buffer
        << rawMsgSize
        << VAsioMsgKind::SubscriptionAnnouncement
        << subscriber;

    _logger->Debug("Announcing subscription for [{}] {}", subscriber.linkName, subscriber.msgTypeName);

    SendIbMsg(std::move(buffer));
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
    assert(_msgBuffer.size() > 0);
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
    if (_currentMsgSize == 0 || _currentMsgSize > 256 * 1024)
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

        MessageBuffer msgBuffer{std::move(_msgBuffer)};
        uint32_t msgSize{0u};
        msgBuffer >> msgSize;
        _ibConnection->OnSocketData(this, std::move(msgBuffer));

        _msgBuffer = std::move(newBuffer);
        _wPos = newWPos;
        _currentMsgSize = 0u;

        DispatchBuffer();
    }
}


} // mw
} // namespace ib
