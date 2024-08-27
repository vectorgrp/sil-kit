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
