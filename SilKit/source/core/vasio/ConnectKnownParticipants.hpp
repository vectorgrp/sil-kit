// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "IIoContext.hpp"
#include "IConnectPeer.hpp"
#include "IConnectionMethods.hpp"
#include "IConnectKnownParticipantsListener.hpp"
#include "ITimer.hpp"
#include "VAsioPeerInfo.hpp"

#include "silkit/services/logging/ILogger.hpp"

#include <atomic>
#include <future>
#include <memory>
#include <string>
#include <unordered_map>

#include "fmt/format.h"


namespace SilKit {
namespace Core {
class VAsioConnection;
class VAsioPeer;
} // namespace Core
} // namespace SilKit


namespace SilKit {
namespace Core {


struct ConnectKnownParticipantsSettings
{
    std::chrono::milliseconds directConnectTimeout{5000};
    std::chrono::milliseconds remoteConnectRequestTimeout{5000};
};


class ConnectKnownParticipants
{
public:
    enum struct PeerEvent
    {
        REMOTE_PARTICIPANT_IS_CONNECTING,
        REMOTE_PARTICIPANT_FAILED_TO_CONNECT,
        REMOTE_PARTICIPANT_ANNOUNCEMENT,
        PARTICIPANT_ANNOUNCEMENT_REPLY,
    };

    enum struct PeerStage
    {
        INVALID = 0,
        /// Connection to this peer has fatally failed
        FAILURE,
        /// Attempting to connect directly
        DIRECT,
        /// Requested remote connection from peer
        REMOTE_CONNECT_REQUESTED,
        /// Remote peer has acknowledged attempting to connect remotely
        REMOTE_IS_CONNECTING,
        /// Waiting for ParticipantAnnouncementReply from peer
        WAITING_FOR_REPLY,
        /// ParticipantAnnouncementReply has been received
        REPLY_RECEIVED,
    };

private:
    enum struct ConnectStage
    {
        INVALID = 0,
        CONNECTING,
        WAITING_FOR_ALL_REPLIES,
        ALL_REPLIES_RECEIVED,
        FAILURE,
    };

    class Peer
        : IConnectPeerListener
        , ITimerListener
    {
    private:
        ConnectKnownParticipants* _manager;
        VAsioPeerInfo _info;
        std::atomic<PeerStage> _peerStage{PeerStage::INVALID};
        std::unique_ptr<IConnectPeer> _directConnectPeer;
        std::unique_ptr<ITimer> _remoteConnectRequestTimer;
        std::string _failureReason;

    public:
        Peer(ConnectKnownParticipants& manager, VAsioPeerInfo info);

        auto GetInfo() const -> const VAsioPeerInfo&;
        auto GetStage() const -> PeerStage;

        void StartConnecting();
        void HandleEvent(PeerEvent event);
        void Shutdown();

        auto Describe() const -> std::string;

    private: // IConnectPeerListener
        void OnConnectPeerSuccess(IConnectPeer&, VAsioPeerInfo peerInfo,
                                  std::unique_ptr<IRawByteStream> stream) override;
        void OnConnectPeerFailure(IConnectPeer&, VAsioPeerInfo) override;

    private: // ITimerListener
        void OnTimerExpired(ITimer& timer) override;

    private:
        void HasFailed(const std::string& reason);
    };

private:
    IIoContext* _ioContext;
    IConnectionMethods* _connectionMethods{nullptr};
    IConnectKnownParticipantsListener* _listener{nullptr};
    ConnectKnownParticipantsSettings _settings;

    SilKit::Services::Logging::ILogger* _logger{nullptr};
    std::promise<std::vector<VAsioPeerInfo>> _knownParticipants;

    std::atomic<ConnectStage> _connectStage{ConnectStage::INVALID};

    mutable std::mutex _mutex{};
    std::unordered_map<std::string, std::unique_ptr<Peer>> _peers;

public:
    explicit ConnectKnownParticipants(IIoContext& ioContext, IConnectionMethods& connectionMethods,
                                      IConnectKnownParticipantsListener& listener,
                                      const ConnectKnownParticipantsSettings& settings);

    void SetLogger(SilKit::Services::Logging::ILogger& logger);

    void SetKnownParticipants(const std::vector<VAsioPeerInfo>& peerInfos);
    void StartConnecting();
    void HandlePeerEvent(const std::string& participantName, PeerEvent event);
    void Shutdown();

    auto Describe() -> std::string;

private:
    auto FindPeerByName(const std::string& name) -> Peer*;
    void UpdateStage();

    friend struct ::fmt::formatter<PeerEvent>;
    friend struct ::fmt::formatter<PeerStage>;
};


} // namespace Core
} // namespace SilKit
