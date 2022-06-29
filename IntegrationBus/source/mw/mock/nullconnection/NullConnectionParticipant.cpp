// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ib/mw/logging/ILogger.hpp"

#include "NullConnectionParticipant.hpp"
#include "CreateParticipant.hpp"
#include "Participant.hpp"
#include "Participant_impl.hpp"

namespace ib {
namespace mw {

namespace {
struct NullConnection
{
    NullConnection(ib::cfg::ParticipantConfiguration /*config*/, std::string /*participantName*/, ib::mw::ParticipantId /*participantId*/, ProtocolVersion) {}

    void SetLogger(logging::ILogger* /*logger*/) {}
    void JoinDomain(uint32_t /*domainId*/) {}

    void JoinDomain(std::string /*registryUri*/) {}

    template<class IbServiceT>
    inline void RegisterIbService(const std::string& /*topicName*/, mw::EndpointId /*endpointId*/, IbServiceT* /*receiver*/) {}

    template <class IbServiceT>
    inline void SetHistoryLengthForLink(const std::string& /*linkName*/, size_t /*history*/, IbServiceT* /*service*/) {}

    template<typename IbMessageT>
    void SendIbMessage(const mw::IIbServiceEndpoint* /*from*/, IbMessageT&& /*msg*/) {}

    template<typename IbMessageT>
    void SendIbMessage(const mw::IIbServiceEndpoint* /*from*/, const std::string& /*target*/, IbMessageT&& /*msg*/) {}

    void OnAllMessagesDelivered(std::function<void()> /*callback*/) {}
    void FlushSendBuffers() {}
    void ExecuteDeferred(std::function<void()> /*callback*/) {}
    void NotifyShutdown() {}

    void RegisterMessageReceiver(std::function<void(IVAsioPeer* /*peer*/, ParticipantAnnouncement)> /*callback*/) {}
    void RegisterPeerShutdownCallback(std::function<void(IVAsioPeer* peer)> /*callback*/) {}
};
} // anonymous namespace
    
auto CreateNullConnectionParticipantImpl(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                                        const std::string& participantName)
    -> std::unique_ptr<IParticipantInternal>
{
    auto&& cfg = ib::mw::ValidateAndSanitizeConfig(participantConfig, participantName);
    return std::make_unique<Participant<NullConnection>>(std::move(cfg), participantName);
}

} // namespace mw
} // namespace ib

