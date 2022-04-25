// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthControllerFacade.hpp"
#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"

namespace ib {
namespace sim {
namespace eth {

EthControllerFacade::EthControllerFacade(mw::IParticipantInternal* participant,
                                         cfg::EthernetController config,
                                         mw::sync::ITimeProvider* timeProvider)
    : _participant{participant}
    , _config{config}
{
    _ethController = std::make_unique<EthController>(participant, config, timeProvider, this);
    _ethControllerProxy = std::make_unique<EthControllerProxy>(participant, config, this);
    _currentController = _ethController.get();
}

// IEthernetController
void EthControllerFacade::Activate()
{
    _currentController->Activate();
}

void EthControllerFacade::Deactivate()
{
    _currentController->Deactivate();
}

// Provided for testing purposes only
auto EthControllerFacade::SendFrameEvent(EthernetFrameEvent msg) -> EthernetTxId
{
    if (IsNetworkSimulated())
    {
        return _ethControllerProxy->SendFrameEvent(std::move(msg));
    }
    else
    {
        return _ethController->SendFrameEvent(std::move(msg));
    }
}

auto EthControllerFacade::SendFrame(EthernetFrame frame) -> EthernetTxId
{
    return _currentController->SendFrame(std::move(frame));
}

void EthControllerFacade::AddFrameHandler(FrameHandler handler)
{
    _ethController->AddFrameHandler(handler);
    _ethControllerProxy->AddFrameHandler(std::move(handler));
}

void EthControllerFacade::AddFrameTransmitHandler(FrameTransmitHandler handler)
{
    _ethController->AddFrameTransmitHandler(handler);
    _ethControllerProxy->AddFrameTransmitHandler(std::move(handler));
}

void EthControllerFacade::AddStateChangeHandler(StateChangeHandler handler)
{
    _ethController->AddStateChangeHandler(handler);
    _ethControllerProxy->AddStateChangeHandler(std::move(handler));
}

void EthControllerFacade::AddBitrateChangeHandler(BitrateChangeHandler handler)
{
    _ethController->AddBitrateChangeHandler(handler);
    _ethControllerProxy->AddBitrateChangeHandler(std::move(handler));
}

// IIbToEthController
void EthControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const EthernetFrameEvent& msg)
{
    if (IsNetworkSimulated())
    {
        if (AllowForwardToProxy(from)) _ethControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        if (AllowForwardToDefault(from)) _ethController->ReceiveIbMessage(from, msg);
    }
}

void EthControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const EthernetFrameTransmitEvent& msg)
{
    if (IsNetworkSimulated() && AllowForwardToProxy(from))
    {
        _ethControllerProxy->ReceiveIbMessage(from, msg);
    }
}

void EthControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const EthernetStatus& msg)
{
    if (IsNetworkSimulated() && AllowForwardToProxy(from))
    {
        _ethControllerProxy->ReceiveIbMessage(from, msg);
    }
}

// ITraceMessageSource
void EthControllerFacade::AddSink(extensions::ITraceMessageSink* sink)
{
    _ethController->AddSink(sink);
    _ethControllerProxy->AddSink(sink);
}

void EthControllerFacade::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
    _ethController->SetServiceDescriptor(serviceDescriptor);
    _ethControllerProxy->SetServiceDescriptor(serviceDescriptor);

    mw::service::IServiceDiscovery* disc = _participant->GetServiceDiscovery();
    disc->RegisterServiceDiscoveryHandler(
        [this, serviceDescriptor]
        (mw::service::ServiceDiscoveryEvent::Type discoveryType, const mw::ServiceDescriptor& remoteServiceDescriptor)
        {
            // check if discovered service is a network simulator (if none is known)
            if (!IsNetworkSimulated())
            {
                // check if received descriptor has a matching simulated link
                if (discoveryType == mw::service::ServiceDiscoveryEvent::Type::ServiceCreated
                    && IsRelevantNetwork(remoteServiceDescriptor))
                {
                    _simulatedLinkDetected = true;
                    _simulatedLink = remoteServiceDescriptor;
                    _currentController = _ethControllerProxy.get();
                }
            }
            else
            {
                if (discoveryType == mw::service::ServiceDiscoveryEvent::Type::ServiceRemoved
                    && IsRelevantNetwork(remoteServiceDescriptor))
                {
                    _simulatedLinkDetected = false;
                    _currentController = _ethController.get();
                }
            }
        });
}

auto EthControllerFacade::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

auto EthControllerFacade::AllowForwardToDefault(const IIbServiceEndpoint* from) const -> bool
{
    const auto& fromDescr = from->GetServiceDescriptor();
    return !(fromDescr.GetParticipantName() == _serviceDescriptor.GetParticipantName()
           && fromDescr.GetServiceId() == _serviceDescriptor.GetServiceId());
}

auto EthControllerFacade::AllowForwardToProxy(const IIbServiceEndpoint* from) const -> bool
{
    const auto& fromDescr = from->GetServiceDescriptor();
    return _simulatedLink.GetParticipantName() == fromDescr.GetParticipantName()
           && _serviceDescriptor.GetServiceId() == fromDescr.GetServiceId();
}

auto EthControllerFacade::IsNetworkSimulated() const -> bool
{
    return _simulatedLinkDetected;
}

auto EthControllerFacade::IsRelevantNetwork(const mw::ServiceDescriptor& remoteServiceDescriptor) const -> bool
{
    return remoteServiceDescriptor.GetServiceType() == ib::mw::ServiceType::Link
           && remoteServiceDescriptor.GetNetworkName() == _serviceDescriptor.GetNetworkName();
}

} // namespace eth
} // namespace sim
} // namespace ib
