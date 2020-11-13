// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioTcpPeer.hpp"
#include "VAsioMsgKind.hpp"
#include "VAsioConnection.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <numeric>

using namespace asio::ip;

#ifdef __linux__

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

static bool SetPlatformSocketOptions(asio::ip::tcp::socket& socket)
{
    int val{1};
    //Disable Delayed Acknowledgments on the receiving side
    int e = setsockopt(socket.native_handle(), IPPROTO_TCP, TCP_QUICKACK,
        (void*)&val, sizeof(val));
    return e == 0;
}

#else  //windows 

static bool SetPlatformSocketOptions(asio::ip::tcp::socket& )
{
    return true;
}

#endif  // __linux__

namespace ib {
namespace mw {

VAsioTcpPeer::VAsioTcpPeer(asio::io_context& io_context, VAsioConnection* ibConnection, logging::ILogger* logger)
    : _socket{io_context}
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

void VAsioTcpPeer::Connect(VAsioPeerInfo peerInfo)
{
    SetInfo(std::move(peerInfo));

    // Resolve the host name using DNS
    tcp::resolver resolver(_socket.get_io_context());
    tcp::resolver::results_type resolverResults;
    try
    {
        resolverResults = resolver.resolve(GetInfo().acceptorHost, std::to_string(GetInfo().acceptorPort));
    }
    catch (asio::system_error& err)
    {
        _logger->Error("Unable to resolve hostname \"{}\": {}", GetInfo().acceptorHost, err.what());
        return;
    }

    for (auto&& resolverEntry : resolverResults)
    {
        try
        {
            _socket.connect(resolverEntry);
            _socket.set_option(asio::ip::tcp::no_delay{true});
            _socket.set_option(asio::socket_base::receive_buffer_size{4096});
            _socket.set_option(asio::socket_base::send_buffer_size{4096});
            _socket.set_option(asio::socket_base::reuse_address{true});
            if (!SetPlatformSocketOptions(_socket))
            {
                _logger->Warn("VasioTcpPeer: cannot set platform-specific socket options (e.g. TCP_QUICKACK)");
            }
            return;
        }
        catch (asio::system_error& /*err*/)
        {
            // reset the socket
            _socket = asio::ip::tcp::socket{_socket.get_io_context()};
        }
    }

    auto errorMsg = fmt::format("Failed to connect to host {}", GetInfo().acceptorHost);
    _logger->Error(errorMsg);
    _logger->Info("Tried the following addresses:");

    for (auto&& resolverEntry : resolverResults)
    {
        _logger->Info(" - {}", tcp::endpoint{ resolverEntry });
    }

    throw std::runtime_error{errorMsg};
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

    _socket.get_io_context().dispatch([this]() { StartAsyncWrite(); });
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
