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
    NullConnection(ib::cfg::Config /*config*/, std::string /*participantName*/, ib::mw::ParticipantId /*participantId*/) {}

    void SetLogger(logging::ILogger* /*logger*/) {}
    void JoinDomain(uint32_t /*domainId*/) {}

    template<class IbServiceT>
    inline void RegisterIbService(const std::string& /*topicName*/, mw::EndpointId /*endpointId*/, IbServiceT* /*receiver*/) {}

    template <class IbServiceT>
    inline void SetHistoryLengthForLink(const std::string& /*linkName*/, size_t /*history*/, IbServiceT* /*service*/) {};

    template<typename IbMessageT>
    void SendIbMessage(const mw::IIbServiceEndpoint* /*from*/, IbMessageT&& /*msg*/) {}

    void OnAllMessagesDelivered(std::function<void()> /*callback*/) {}
    void FlushSendBuffers() {}
    void ExecuteDeferred(std::function<void()> /*callback*/) {}
    void NotifyShutdown() {}
};
} // anonymous namespace
    
auto CreateNullConnectionComAdapterImpl(ib::cfg::Config config, const std::string& participantName) -> std::unique_ptr<IComAdapterInternal>
{
    return std::make_unique<ComAdapter<NullConnection>>(std::move(config), participantName);
}

} // mw
} // namespace ib

