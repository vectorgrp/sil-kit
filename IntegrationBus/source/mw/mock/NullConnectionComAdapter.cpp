// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "NullConnectionComAdapter.hpp"

#include "ComAdapter.hpp"
#include "ComAdapter_impl.hpp"

namespace ib {
namespace mw {

namespace {
struct NullConnection
{
    NullConnection(ib::cfg::Config /*config*/, std::string /*participantName*/) {};

    void joinDomain(uint32_t /*domainId*/) {};

    template<class IbServiceT>
    inline void RegisterIbService(const std::string& /*topicName*/, EndpointId /*endpointId*/, IbServiceT* /*receiver*/) {};

    template<typename IbMessageT>
    void SendIbMessageImpl(EndpointAddress /*from*/, IbMessageT&& /*msg*/) {};

    void WaitForMessageDelivery() {};
    void FlushSendBuffers() {};

    void Run() {};
    void Stop() {};
};
} // anonymous namespace
    
auto CreateNullConnectionComAdapterImpl(ib::cfg::Config config, const std::string& participantName) -> std::unique_ptr<IComAdapterInternal>
{
    return std::make_unique<ComAdapter<NullConnection>>(std::move(config), participantName);
}

} // mw
} // namespace ib

