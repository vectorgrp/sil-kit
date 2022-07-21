// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "silkit/services/logging/ILogger.hpp"

#include "NullConnectionParticipant.hpp"
#include "CreateParticipant.hpp"
#include "Participant.hpp"
#include "Participant_impl.hpp"
#include "CreateParticipant_impl.hpp"

namespace SilKit {
namespace Core {

namespace {
struct NullConnection
{
    NullConnection(SilKit::Config::ParticipantConfiguration /*config*/, std::string /*participantName*/,
                   SilKit::ParticipantId /*participantId*/,
                   SilKit::Core::Orchestration::ITimeProvider* /*timeProvider*/, ProtocolVersion)
    {
    }

    void SetLogger(Services::Logging::ILogger* /*logger*/) {}
    void SetTimeSyncService(Orchestration::TimeSyncService* /*timeSyncService*/) {}
    void JoinSimulation(std::string /*registryUri*/) {}

    template<class SilKitServiceT>
    inline void RegisterSilKitService(const std::string& /*topicName*/, Core::EndpointId /*endpointId*/, SilKitServiceT* /*receiver*/) {}

    template <class SilKitServiceT>
    inline void SetHistoryLengthForLink(const std::string& /*linkName*/, size_t /*history*/, SilKitServiceT* /*service*/) {}

    template<typename SilKitMessageT>
    void SendMsg(const Core::IServiceEndpoint* /*from*/, SilKitMessageT&& /*msg*/) {}

    template<typename SilKitMessageT>
    void SendMsg(const Core::IServiceEndpoint* /*from*/, const std::string& /*target*/, SilKitMessageT&& /*msg*/) {}

    void OnAllMessagesDelivered(std::function<void()> /*callback*/) {}
    void FlushSendBuffers() {}
    void ExecuteDeferred(std::function<void()> /*callback*/) {}
    void NotifyShutdown() {}

    void RegisterMessageReceiver(std::function<void(IVAsioPeer* /*peer*/, ParticipantAnnouncement)> /*callback*/) {}
    void RegisterPeerShutdownCallback(std::function<void(IVAsioPeer* peer)> /*callback*/) {}
};
} // anonymous namespace
    
auto CreateNullConnectionParticipantImpl(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                                         const std::string& participantName) -> std::unique_ptr<IParticipantInternal>
{
    return CreateParticipantImplInternal<NullConnection>(std::move(participantConfig), participantName);
}

} // namespace Core
} // namespace SilKit

