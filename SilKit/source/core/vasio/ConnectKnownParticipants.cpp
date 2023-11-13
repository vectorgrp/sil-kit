// Copyright (c) 2022 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#include "ConnectKnownParticipants.hpp"

#include "ILogger.hpp"
#include "VAsioConnection.hpp"
#include "VAsioPeer.hpp"
#include "util/TracingMacros.hpp"

#include <chrono>
#include <memory>

#include "fmt/format.h"


#if SILKIT_ENABLE_TRACING_INSTRUMENTATION_VAsioConnection
#    define SILKIT_TRACE_METHOD_(logger, ...) SILKIT_TRACE_METHOD(logger, __VA_ARGS__)
#else
#    define SILKIT_TRACE_METHOD_(...)
#endif


using namespace std::chrono_literals;

namespace Log = SilKit::Services::Logging;


namespace SilKit {
namespace Core {


ConnectKnownParticipants::ConnectKnownParticipants(IIoContext& ioContext, IConnectionMethods& connectionMethods,
                                                   IConnectKnownParticipantsListener& listener,
                                                   const ConnectKnownParticipantsSettings& settings)
    : _ioContext{&ioContext}
    , _connectionMethods{&connectionMethods}
    , _listener{&listener}
    , _settings{settings}
{
    SILKIT_TRACE_METHOD_(_logger, "({})", static_cast<const void*>(&vAsioConnection));
}


void ConnectKnownParticipants::SetLogger(SilKit::Services::Logging::ILogger& logger)
{
    SILKIT_ASSERT(_logger == nullptr);
    _logger = &logger;
}


void ConnectKnownParticipants::SetKnownParticipants(const std::vector<VAsioPeerInfo>& peerInfos)
{
    SILKIT_TRACE_METHOD_(_logger, "({})", peerInfos.size());

    _knownParticipants.set_value(peerInfos);
}


void ConnectKnownParticipants::StartConnecting()
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    SILKIT_ASSERT(_connectStage == ConnectStage::INVALID);
    _connectStage = ConnectStage::CONNECTING;

    // wait for the known participants to be set
    auto knownParticipants{_knownParticipants.get_future()};

    {
        std::lock_guard<decltype(_mutex)> lock{_mutex};

        // create all peer state trackers
        for (const auto& peerInfo : knownParticipants.get())
        {
            _peers.emplace(peerInfo.participantName, std::make_unique<Peer>(*this, peerInfo));
        }
    }

    // initiate all direct connection attempts
    for (const auto& pair : _peers)
    {
        const auto& peer{pair.second};
        peer->StartConnecting();
    }

    UpdateStage();
}

void ConnectKnownParticipants::HandlePeerEvent(const std::string& participantName, PeerEvent event)
{
    SILKIT_TRACE_METHOD_(_logger, "({}, ...)", participantName);

    auto peer{FindPeerByName(participantName)};
    if (peer == nullptr)
    {
        Log::Warn(_logger, "Ignoring event '{}' for peer '{}'", event, participantName);
        return;
    }

    peer->HandleEvent(event);
}


auto ConnectKnownParticipants::FindPeerByName(const std::string& name) -> Peer*
{
    SILKIT_TRACE_METHOD_(_logger, "({})", name);

    std::lock_guard<decltype(_mutex)> lock{_mutex};

    auto it{_peers.find(name)};
    if (it == _peers.end())
    {
        return nullptr;
    }

    return it->second.get();
}


void ConnectKnownParticipants::Shutdown()
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    std::lock_guard<decltype(_mutex)> lock{_mutex};

    for (const auto& pair : _peers)
    {
        const auto& peer{pair.second};
        peer->Shutdown();
    }
}


auto ConnectKnownParticipants::Describe() -> std::string
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    std::lock_guard<decltype(_mutex)> lock{_mutex};

    if (_peers.empty())
    {
        return "no participants";
    }

    std::vector<std::string> descriptions;
    std::transform(_peers.begin(), _peers.end(), std::back_inserter(descriptions), [](const auto& pair) {
        const auto& peer{pair.second};
        return peer->Describe();
    });

    return fmt::format("{}", fmt::join(descriptions, "; "));
}


void ConnectKnownParticipants::UpdateStage()
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    if (_connectStage == ConnectStage::INVALID)
    {
        return;
    }

    if (_connectStage == ConnectStage::ALL_REPLIES_RECEIVED || _connectStage == ConnectStage::FAILURE)
    {
        return;
    }

    const bool anyPeerFailed{[this] {
        std::lock_guard<decltype(_mutex)> lock{_mutex};
        return std::any_of(_peers.begin(), _peers.end(), [](const auto& pair) {
            const auto& peer{pair.second};
            return peer->GetStage() == PeerStage::FAILURE;
        });
    }()};

    if (anyPeerFailed)
    {
        _connectStage = ConnectStage::FAILURE;
        _listener->OnConnectKnownParticipantsFailure(*this);

        return;
    }

    if (_connectStage == ConnectStage::CONNECTING)
    {
        const bool allWaitingForReplies{[this] {
            std::lock_guard<decltype(_mutex)> lock{_mutex};
            return std::all_of(_peers.begin(), _peers.end(), [](const auto& pair) {
                const auto& peer{pair.second};
                return peer->GetStage() >= PeerStage::WAITING_FOR_REPLY;
            });
        }()};

        if (!allWaitingForReplies)
        {
            return;
        }

        _connectStage = ConnectStage::WAITING_FOR_ALL_REPLIES;
        _listener->OnConnectKnownParticipantsWaitingForAllReplies(*this);
    }

    if (_connectStage == ConnectStage::WAITING_FOR_ALL_REPLIES)
    {
        const bool allRepliesReceived{[this] {
            std::lock_guard<decltype(_mutex)> lock{_mutex};
            return std::all_of(_peers.begin(), _peers.end(), [](const auto& pair) {
                const auto& peer{pair.second};
                return peer->GetStage() >= PeerStage::REPLY_RECEIVED;
            });
        }()};

        if (!allRepliesReceived)
        {
            return;
        }

        _connectStage = ConnectStage::ALL_REPLIES_RECEIVED;
        _listener->OnConnectKnownParticipantsAllRepliesReceived(*this);
    }
}


// ConnectionManager::Peer : IConnectPeerListener


ConnectKnownParticipants::Peer::Peer(ConnectKnownParticipants& manager, VAsioPeerInfo info)
    : _manager{&manager}
    , _info{std::move(info)}
{
}


auto ConnectKnownParticipants::Peer::GetStage() const -> PeerStage
{
    return _peerStage;
}

auto ConnectKnownParticipants::Peer::GetInfo() const -> const VAsioPeerInfo&
{
    return _info;
}


void ConnectKnownParticipants::Peer::StartConnecting()
{
    SILKIT_TRACE_METHOD_(_manager->_logger, "()");

    _peerStage = PeerStage::DIRECT;

    _directConnectPeer = _manager->_connectionMethods->MakeConnectPeer(_info);
    _directConnectPeer->SetListener(*this);
    _directConnectPeer->AsyncConnect(1, _manager->_settings.directConnectTimeout);
}

void ConnectKnownParticipants::Peer::HandleEvent(PeerEvent event)
{
    SILKIT_TRACE_METHOD_(_manager->_logger, "({})", event);

    switch (event)
    {
    case PeerEvent::REMOTE_PARTICIPANT_IS_CONNECTING:
    {
        if (_peerStage != PeerStage::REMOTE_CONNECT_REQUESTED)
        {
            Log::Warn(_manager->_logger,
                      "Ignoring unexpected remote participant connecting notification from peer '{}' in stage {}",
                      _info.participantName, _peerStage.load());
            return;
        }

        _peerStage = PeerStage::REMOTE_IS_CONNECTING;
        _manager->UpdateStage();

        return;
    }
    case PeerEvent::REMOTE_PARTICIPANT_FAILED_TO_CONNECT:
    {
        if (_peerStage != PeerStage::REMOTE_CONNECT_REQUESTED && _peerStage != PeerStage::REMOTE_IS_CONNECTING)
        {
            Log::Warn(
                _manager->_logger,
                "Ignoring unexpected remote participant connection failure notification from peer '{}' in stage {}",
                _info.participantName, _peerStage.load());
            return;
        }

        // attempt to connect via proxy, which immediately sends our ParticipantAnnouncement via the proxy
        _peerStage = PeerStage::WAITING_FOR_REPLY;
        if (_manager->_connectionMethods->TryProxyConnect(_info))
        {
            _manager->UpdateStage();
            return;
        }

        HasFailed("because direct connection failed, the remote was unable to connect, and proxy-connection is not "
                  "available");
        return;
    }
    case PeerEvent::REMOTE_PARTICIPANT_ANNOUNCEMENT:
    {
        if (_peerStage != PeerStage::REMOTE_CONNECT_REQUESTED && _peerStage != PeerStage::REMOTE_IS_CONNECTING)
        {
            Log::Warn(_manager->_logger,
                      "Ignoring unexpected remote participant announcement from peer '{}' in stage {}",
                      _info.participantName, _peerStage.load());
            return;
        }

        _peerStage = PeerStage::WAITING_FOR_REPLY;
        _manager->UpdateStage();

        break;
    }
    case PeerEvent::PARTICIPANT_ANNOUNCEMENT_REPLY:
    {
        if (_peerStage != PeerStage::WAITING_FOR_REPLY)
        {
            Log::Warn(_manager->_logger, "Ignoring unexpected reply from peer '{}' in stage {}", _info.participantName,
                      _peerStage.load());
            return;
        }

        _peerStage = PeerStage::REPLY_RECEIVED;
        _manager->UpdateStage();

        return;
    }
    }
}

void ConnectKnownParticipants::Peer::Shutdown()
{
    SILKIT_TRACE_METHOD_(_manager->_logger, "()");

    if (_directConnectPeer != nullptr)
    {
        _directConnectPeer->Shutdown();
    }

    if (_remoteConnectRequestTimer != nullptr)
    {
        _remoteConnectRequestTimer->Shutdown();
    }
}

auto ConnectKnownParticipants::Peer::Describe() const -> std::string
{
    std::string buffer;
    auto it{std::back_inserter(buffer)};

    it = fmt::format_to(it, "{} ({}) ", _info.participantName, fmt::join(_info.acceptorUris, ", "));

    switch (_peerStage)
    {
    case PeerStage::INVALID:
        fmt::format_to(it, "has not started connecting");
        break;
    case PeerStage::FAILURE:
        fmt::format_to(it, "has failed{}", _failureReason);
        break;
    case PeerStage::DIRECT:
        fmt::format_to(it, "is connecting directly");
        break;
    case PeerStage::REMOTE_CONNECT_REQUESTED:
        fmt::format_to(it, "has requested remote connection");
        break;
    case PeerStage::REMOTE_IS_CONNECTING:
        fmt::format_to(it, "is connecting remotely");
        break;
    case PeerStage::WAITING_FOR_REPLY:
        fmt::format_to(it, "is waiting for reply");
        break;
    case PeerStage::REPLY_RECEIVED:
        fmt::format_to(it, "is connected");
        break;
    }

    return buffer;
}


void ConnectKnownParticipants::Peer::OnConnectPeerSuccess(IConnectPeer&, VAsioPeerInfo peerInfo,
                                                          std::unique_ptr<IRawByteStream> stream)
{
    SILKIT_TRACE_METHOD_(_manager->_logger, "(..., ...)");

    // destroy the peer connection object
    _directConnectPeer.reset();

    auto vAsioPeer{_manager->_connectionMethods->MakeVAsioPeer(std::move(stream))};
    vAsioPeer->SetInfo(std::move(peerInfo));

    _peerStage = PeerStage::WAITING_FOR_REPLY;
    _manager->_connectionMethods->HandleConnectedPeer(vAsioPeer.get());
    _manager->_connectionMethods->AddPeer(std::move(vAsioPeer));
    _manager->UpdateStage();
}

void ConnectKnownParticipants::Peer::OnConnectPeerFailure(IConnectPeer&, VAsioPeerInfo)
{
    SILKIT_TRACE_METHOD_(_manager->_logger, "(..., ...)");

    // destroy the peer connection object
    _directConnectPeer.reset();

    // attempt to request remote connection
    _peerStage = PeerStage::REMOTE_CONNECT_REQUESTED;
    if (_manager->_connectionMethods->TryRemoteConnectRequest(_info))
    {
        _remoteConnectRequestTimer = _manager->_ioContext->MakeTimer();
        _remoteConnectRequestTimer->SetListener(*this);
        _remoteConnectRequestTimer->AsyncWaitFor(_manager->_settings.remoteConnectRequestTimeout);
        _manager->UpdateStage();
        return;
    }

    // attempt to connect via proxy, which immediately sends our ParticipantAnnouncement via the proxy
    _peerStage = PeerStage::WAITING_FOR_REPLY;
    if (_manager->_connectionMethods->TryProxyConnect(_info))
    {
        _manager->UpdateStage();
        return;
    }

    HasFailed(
        "because direct connection failed, and both remote-connect, as well as proxy-connection are not available");
}


void ConnectKnownParticipants::Peer::OnTimerExpired(VSilKit::ITimer&)
{
    SILKIT_TRACE_METHOD_(_manager->_logger, "(...)");

    if (_peerStage != PeerStage::REMOTE_CONNECT_REQUESTED)
    {
        Log::Debug(_manager->_logger, "Ignoring expired remote connection request timer for {} in stage {}",
                   _info.participantName, _peerStage.load());
        return;
    }

    _peerStage = PeerStage::WAITING_FOR_REPLY;
    if (_manager->_connectionMethods->TryProxyConnect(_info))
    {
        _manager->UpdateStage();
        return;
    }

    HasFailed("because direct connection failed, remote connections timed out, and proxy-connection is not available");
}


void ConnectKnownParticipants::Peer::HasFailed(const std::string& reason)
{
    SILKIT_TRACE_METHOD_(_logger, "({})", reason);

    Log::Error(_manager->_logger, "Failed to connect to '{}' {}", _info.participantName, reason);

    _failureReason = " ";
    _failureReason.append(reason);

    _peerStage = PeerStage::FAILURE;
    _manager->UpdateStage();
}


} // namespace Core
} // namespace SilKit


template <>
struct fmt::formatter<::SilKit::Core::ConnectKnownParticipants::PeerEvent> : formatter<string_view>
{
    using Event = ::SilKit::Core::ConnectKnownParticipants::PeerEvent;

    auto format(const Event& event, format_context& ctx) const -> format_context::iterator
    {
        switch (event)
        {
        case Event::REMOTE_PARTICIPANT_IS_CONNECTING:
            return formatter<string_view>::format("REMOTE_PARTICIPANT_IS_CONNECTING", ctx);
        case Event::REMOTE_PARTICIPANT_FAILED_TO_CONNECT:
            return formatter<string_view>::format("REMOTE_PARTICIPANT_FAILED_TO_CONNECT", ctx);
        case Event::REMOTE_PARTICIPANT_ANNOUNCEMENT:
            return formatter<string_view>::format("REMOTE_PARTICIPANT_ANNOUNCEMENT", ctx);
        case Event::PARTICIPANT_ANNOUNCEMENT_REPLY:
            return formatter<string_view>::format("PARTICIPANT_ANNOUNCEMENT_REPLY", ctx);
        default:
            return fmt::format_to(ctx.out(), "ConnectionManager::PeerEvent({})",
                                  static_cast<std::underlying_type_t<Event>>(event));
        }
    }
};

template <>
struct fmt::formatter<::SilKit::Core::ConnectKnownParticipants::PeerStage> : formatter<string_view>
{
    using Stage = ::SilKit::Core::ConnectKnownParticipants::PeerStage;

    auto format(const Stage& stage, format_context& ctx) const -> format_context::iterator
    {
        switch (stage)
        {
        case Stage::INVALID:
            return formatter<string_view>::format("INVALID", ctx);
        case Stage::DIRECT:
            return formatter<string_view>::format("DIRECT", ctx);
        case Stage::REMOTE_CONNECT_REQUESTED:
            return formatter<string_view>::format("REMOTE_CONNECT_REQUESTED", ctx);
        case Stage::REMOTE_IS_CONNECTING:
            return formatter<string_view>::format("REMOTE_IS_CONNECTING", ctx);
        case Stage::WAITING_FOR_REPLY:
            return formatter<string_view>::format("WAITING_FOR_REPLY", ctx);
        case Stage::REPLY_RECEIVED:
            return formatter<string_view>::format("REPLY_RECEIVED", ctx);
        default:
            return fmt::format_to(ctx.out(), "ConnectionManager::PeerStage({})",
                                  static_cast<std::underlying_type_t<Stage>>(stage));
        }
    }
};


#undef SILKIT_TRACE_METHOD_
