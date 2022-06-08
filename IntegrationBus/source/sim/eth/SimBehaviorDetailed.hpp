// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbToEthController.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"

#include "ISimBehavior.hpp"

namespace ib {
namespace sim {
namespace eth {

class EthController;

class SimBehaviorDetailed : public ISimBehavior
{
public:
    SimBehaviorDetailed(mw::IParticipantInternal* participant, EthController* ethController,
                       const mw::ServiceDescriptor& serviceDescriptor);

    void SendIbMessage(EthernetFrameEvent&& msg) override;
    void SendIbMessage(EthernetSetMode&& msg) override;
    void OnReceiveAck(const EthernetFrameTransmitEvent& msg) override;
    
    auto AllowReception(const mw::IIbServiceEndpoint* from) const -> bool override;

    void SetSimulatedLink(const mw::ServiceDescriptor& simulatedLink);

private:
    template <typename MsgT>
    void SendIbMessageImpl(MsgT&& msg);

    mw::IParticipantInternal* _participant{nullptr};
    const mw::IIbServiceEndpoint* _parentServiceEndpoint{nullptr};
    const mw::ServiceDescriptor* _parentServiceDescriptor{nullptr};
    mw::ServiceDescriptor _simulatedLink;
    extensions::Tracer _tracer;
    std::map<EthernetTxId, EthernetFrame> _transmittedMessages;
};


} // namespace eth
} // namespace sim
} // namespace ib
