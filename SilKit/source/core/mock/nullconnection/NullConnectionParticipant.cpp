// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/services/logging/ILogger.hpp"

#include "NullConnectionParticipant.hpp"
#include "Participant.hpp"
#include "Participant_impl.hpp"
#include "CreateParticipantT.hpp"

namespace SilKit {
namespace Core {

namespace {
struct NullConnection
{
    NullConnection(SilKit::Core::IParticipantInternal*, VSilKit::IMetricsManager*,
                   SilKit::Config::ParticipantConfiguration /*config*/, std::string /*participantName*/,
                   SilKit::Core::ParticipantId /*participantId*/,
                   SilKit::Core::Orchestration::ITimeProvider* /*timeProvider*/, ProtocolVersion)
    {
    }

    void SetLogger(Services::Logging::ILogger* /*logger*/) {}
    void SetLoggerInternal(Services::Logging::ILoggerInternal* /*logger*/) {}
    void SetTimeSyncService(Orchestration::TimeSyncService* /*timeSyncService*/) {}
    void JoinSimulation(std::string /*registryUri*/) {}

    template <class SilKitServiceT>
    inline void RegisterSilKitService(SilKitServiceT* /*service*/)
    {
    }

    template <class SilKitServiceT>
    inline void SetHistoryLengthForLink(size_t /*history*/, SilKitServiceT* /*service*/)
    {
    }

    template <typename SilKitMessageT>
    void SendMsg(const Core::IServiceEndpoint* /*from*/, SilKitMessageT&& /*msg*/)
    {
    }

    template <typename SilKitMessageT>
    void SendMsg(const Core::IServiceEndpoint* /*from*/, const std::string& /*target*/, SilKitMessageT&& /*msg*/)
    {
    }

    void OnAllMessagesDelivered(std::function<void()> /*callback*/) {}
    void FlushSendBuffers() {}
    void ExecuteDeferred(std::function<void()> /*callback*/) {}
    void NotifyShutdown() {}
    void EnableAggregation() {}

    void RegisterMessageReceiver(std::function<void(IVAsioPeer* /*peer*/, ParticipantAnnouncement)> /*callback*/) {}
    void RegisterPeerShutdownCallback(std::function<void(IVAsioPeer* peer)> /*callback*/) {}

    void AddAsyncSubscriptionsCompletionHandler(std::function<void()> /*completionHandler*/) {}

    size_t GetNumberOfConnectedParticipants()
    {
        return 0;
    }

    std::vector<std::string> GetConnectedParticipantsNames()
    {
        std::vector<std::string> tmp;
        return tmp;
    }

    size_t GetNumberOfRemoteReceivers(const IServiceEndpoint* /*service*/, const std::string& /*msgTypeName*/)
    {
        return 0;
    };
    std::vector<std::string> GetParticipantNamesOfRemoteReceivers(const IServiceEndpoint* /*service*/,
                                                                  const std::string& /*msgTypeName*/)
    {
        return {};
    };

    bool ParticipantHasCapability(const std::string& /*participantName*/, const std::string& /*capability*/) const
    {
        return true;
    }
};
} // anonymous namespace

auto CreateNullConnectionParticipantImpl(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                                         const std::string& participantName) -> std::unique_ptr<IParticipantInternal>
{
    return CreateParticipantT<NullConnection>(std::move(participantConfig), participantName,
                                              "silkit://null.connection.silkit:0");
}

} // namespace Core
} // namespace SilKit
