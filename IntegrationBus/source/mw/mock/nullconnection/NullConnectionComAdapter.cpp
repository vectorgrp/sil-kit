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
    NullConnection(ib::cfg::Config /*config*/, std::string /*participantName*/, ib::mw::ParticipantId /*participantId*/) {};

    void SetLogger(logging::ILogger* /*logger*/) {};
    void JoinDomain(uint32_t /*domainId*/) {};

    template<class IbServiceT>
    inline void RegisterIbService(const std::string& /*topicName*/, mw::EndpointId /*endpointId*/, IbServiceT* /*receiver*/) {};

    template<typename IbMessageT>
    void SendIbMessage(mw::EndpointAddress /*from*/, IbMessageT&& /*msg*/) {};

    void OnAllMessagesDelivered(std::function<void(void)> /*callback*/) {};
    void FlushSendBuffers() {};
};
} // anonymous namespace
    
auto CreateNullConnectionComAdapterImpl(ib::cfg::Config config, const std::string& participantName) -> std::unique_ptr<IComAdapterInternal>
{
    return std::make_unique<ComAdapter<NullConnection>>(std::move(config), participantName);
}

} // mw
} // namespace ib

