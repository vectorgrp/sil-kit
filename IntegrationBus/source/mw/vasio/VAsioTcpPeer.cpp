// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioTcpPeer.hpp"
#include "VAsioMsgKind.hpp"
#include "VAsioConnection.hpp"

#include <iostream>
#include <iomanip>

using namespace asio::ip;

namespace ib {
namespace mw {

VAsioTcpPeer::VAsioTcpPeer(asio::io_context& io_context, VAsioConnection* ibConnection)
    : _socket{io_context}
    , _ibConnection{ibConnection}
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
        std::cout << "INFO: Shutting down connection to " << _info.participantName << std::endl;
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
        std::cerr
            << "ERROR: Unable to resolve hostname \"" << GetInfo().acceptorHost << "\""
            << ": " << err.what()
            << "\n";
        return;
    }


    for (auto&& resolverEntry : resolverResults)
    {
        try
        {
            _socket.connect(resolverEntry);
            return;
        }
        catch (asio::system_error& /*err*/)
        {
            // reset the socket
            _socket = asio::ip::tcp::socket{_socket.get_io_context()};
        }
    }

    std::cerr << "ERROR: failed to connect to host " << GetInfo().acceptorHost << ". Tried the following addresses:\n";
    for (auto&& resolverEntry : resolverResults)
    {
        std::cerr << " - " << tcp::endpoint{resolverEntry} << "\n";
    }
}

void VAsioTcpPeer::SendIbMsg(MessageBuffer buffer)
{
    auto sendBuffer = buffer.ReleaseStorage();
    if (sendBuffer.size() > std::numeric_limits<uint32_t>::max())
        throw std::runtime_error{"Message is too large"};

    *reinterpret_cast<uint32_t*>(sendBuffer.data()) = static_cast<uint32_t>(sendBuffer.size());

    std::unique_lock<std::mutex> lock{ _sendingQueueLock };
    if (_sendingQueue.size() >= _sendingQueueMaxSize)
    {
        std::cout << "Sending Queue is full. Dropping messages!" << std::endl;
    }
    else
    {
        _sendingQueue.push(std::move(sendBuffer));
    }
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
        << VAsioMsgKind::AnnounceSubscription
        << subscriber;

    std::cout << "INFO: Announcing subscription for: [" << subscriber.linkName << "] " << subscriber.msgTypeName << "\n";

    SendIbMsg(std::move(buffer));
}

void VAsioTcpPeer::StartAsyncRead()
{
    _currentMsgSize = 0u;

    _msgBuffer.resize(128);
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
                _msgBuffer.resize(128);
            // and restart the async read operation
            ReadSomeAsync();
            return;
        }
    }

    // validate the received size
    if (_currentMsgSize == 0 || _currentMsgSize > 256 * 1024)
    {
        std::cout << "Received invalid Message Size: " << _currentMsgSize << "\n";
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
