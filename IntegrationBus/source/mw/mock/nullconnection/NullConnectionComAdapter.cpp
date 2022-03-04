// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "NullConnectionComAdapter.hpp"

#include "ib/mw/logging/ILogger.hpp"

#include "ComAdapter.hpp"
#include "ComAdapter_impl.hpp"

namespace ib {
namespace mw {

namespace {
struct NullConnection
{
    NullConnection(std::shared_ptr<ib::cfg::v1::datatypes::ParticipantConfiguration> /*config*/, std::string /*participantName*/, ib::mw::ParticipantId /*participantId*/) {}

    void SetLogger(logging::ILogger* /*logger*/) {}
    void JoinDomain(uint32_t /*domainId*/) {}

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
    void RegisterPeerShutdownCallback(std::function<void(IVAsioPeer* peer)> /*callback*/) {}


};
} // anonymous namespace
    
auto CreateNullConnectionComAdapterImpl(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                                        const std::string& participantName, bool isSynchronized)
    -> std::unique_ptr<IComAdapterInternal>
{
    return std::make_unique<ComAdapter<NullConnection>>(std::move(participantConfig), participantName, isSynchronized);
}

} // namespace mw
} // namespace ib

