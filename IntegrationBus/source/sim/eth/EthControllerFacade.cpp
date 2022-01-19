// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthControllerFacade.hpp"
#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"

namespace ib {
namespace sim {
namespace eth {

EthControllerFacade::EthControllerFacade(mw::IComAdapterInternal* comAdapter, cfg::EthernetController config, mw::sync::ITimeProvider* timeProvider)
  : _comAdapter{ comAdapter }
{
    _ethController = std::make_unique<EthController>(comAdapter, config, timeProvider);
    _ethControllerProxy = std::make_unique<EthControllerProxy>(comAdapter, config);
    _currentController = _ethController.get();
}

// IEthController
void EthControllerFacade::Activate()
{
    _currentController->Activate();
}

void EthControllerFacade::Deactivate()
{
    _currentController->Deactivate();
}

auto EthControllerFacade::SendMessage(EthMessage msg) -> EthTxId
{
    return _currentController->SendMessage(std::move(msg));
}

auto EthControllerFacade::SendFrame(EthFrame frame) -> EthTxId
{
    return _currentController->SendFrame(std::move(frame));
}

auto EthControllerFacade::SendFrame(EthFrame frame, std::chrono::nanoseconds timestamp) -> EthTxId
{
    return _currentController->SendFrame(std::move(frame), std::move(timestamp));
}

void EthControllerFacade::RegisterReceiveMessageHandler(ReceiveMessageHandler handler)
{
    _ethController->RegisterReceiveMessageHandler(handler);
    _ethControllerProxy->RegisterReceiveMessageHandler(handler);
}

void EthControllerFacade::RegisterMessageAckHandler(MessageAckHandler handler)
{
    _ethController->RegisterMessageAckHandler(handler);
    _ethControllerProxy->RegisterMessageAckHandler(handler);
}

void EthControllerFacade::RegisterStateChangedHandler(StateChangedHandler handler)
{
    _ethController->RegisterStateChangedHandler(handler);
    _ethControllerProxy->RegisterStateChangedHandler(handler);
}

void EthControllerFacade::RegisterBitRateChangedHandler(BitRateChangedHandler handler)
{
    _ethController->RegisterBitRateChangedHandler(handler);
    _ethControllerProxy->RegisterBitRateChangedHandler(handler);
}

// IIbToEthController
void EthControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const EthMessage& msg)
{
    if (IsLinkSimulated())
    {
        if (ProxyFilter(from)) _ethControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        _ethController->ReceiveIbMessage(from, msg);
    }
}

void EthControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const EthTransmitAcknowledge& msg)
{
    if (IsLinkSimulated() && ProxyFilter(from))
    {
        _ethControllerProxy->ReceiveIbMessage(from, msg);
    }
}

void EthControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const EthStatus& msg)
{
    if (IsLinkSimulated() && ProxyFilter(from))
    {
        _ethControllerProxy->ReceiveIbMessage(from, msg);
    }
}

void EthControllerFacade::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    // TODO remove support
    _ethController->SetEndpointAddress(endpointAddress);
    _ethControllerProxy->SetEndpointAddress(endpointAddress);
}

auto EthControllerFacade::EndpointAddress() const -> const mw::EndpointAddress&
{
    // TODO remove!
    if (IsLinkSimulated())
    {
        return _ethControllerProxy->EndpointAddress();
    }
    else
    {
        return _ethController->EndpointAddress();
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

    mw::service::IServiceDiscovery* disc = _comAdapter->GetServiceDiscovery();
    disc->RegisterServiceDiscoveryHandler(
        [this, serviceDescriptor]
        (mw::service::ServiceDiscoveryEvent::Type discoveryType, const mw::ServiceDescriptor& remoteServiceDescriptor)
        {
            // check if discovered service is a network simulator (if none is known)
            if (!IsLinkSimulated())
            {
                // check if received descriptor has a matching simulated link
                if (discoveryType == mw::service::ServiceDiscoveryEvent::Type::ServiceCreated
                    && remoteServiceDescriptor.type == serviceDescriptor.type
                    && remoteServiceDescriptor.linkName == serviceDescriptor.linkName
                    && remoteServiceDescriptor.isLinkSimulated)
                {
                    _remoteBusSimulator = remoteServiceDescriptor;
                    _currentController = _ethControllerProxy.get();
                }
            }
            else
            {
                if (discoveryType == mw::service::ServiceDiscoveryEvent::Type::ServiceRemoved
                    && remoteServiceDescriptor.type == serviceDescriptor.type
                    && remoteServiceDescriptor.linkName == serviceDescriptor.linkName)
                {
                    _remoteBusSimulator.isLinkSimulated = false;
                    _currentController = _ethController.get();
                }
            }
        });
}

auto EthControllerFacade::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

auto EthControllerFacade::DefaultFilter(const IIbServiceEndpoint* from) const -> bool
{
    return true;
}

auto EthControllerFacade::ProxyFilter(const IIbServiceEndpoint* from) const -> bool
{
    return _remoteBusSimulator.participantName == from->GetServiceDescriptor().participantName;
}

auto EthControllerFacade::IsLinkSimulated() const -> bool
{
    return _remoteBusSimulator.isLinkSimulated;
}

} // namespace eth
} // namespace sim
} // namespace ib
