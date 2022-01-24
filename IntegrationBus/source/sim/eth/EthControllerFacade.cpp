// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthControllerFacade.hpp"
#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"

namespace ib {
namespace sim {
namespace eth {

EthControllerFacade::EthControllerFacade(mw::IComAdapterInternal* comAdapter, cfg::EthernetController config, mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _config{config}
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
    _ethControllerProxy->RegisterReceiveMessageHandler(std::move(handler));
}

void EthControllerFacade::RegisterMessageAckHandler(MessageAckHandler handler)
{
    _ethController->RegisterMessageAckHandler(handler);
    _ethControllerProxy->RegisterMessageAckHandler(std::move(handler));
}

void EthControllerFacade::RegisterStateChangedHandler(StateChangedHandler handler)
{
    _ethController->RegisterStateChangedHandler(handler);
    _ethControllerProxy->RegisterStateChangedHandler(std::move(handler));
}

void EthControllerFacade::RegisterBitRateChangedHandler(BitRateChangedHandler handler)
{
    _ethController->RegisterBitRateChangedHandler(handler);
    _ethControllerProxy->RegisterBitRateChangedHandler(std::move(handler));
}

// IIbToEthController
void EthControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const EthMessage& msg)
{
    if (IsLinkSimulated())
    {
        if (AllowForwardToProxy(from)) _ethControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        if (AllowForwardToDefault(from)) _ethController->ReceiveIbMessage(from, msg);
    }
}

void EthControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const EthTransmitAcknowledge& msg)
{
    if (IsLinkSimulated() && AllowForwardToProxy(from))
    {
        _ethControllerProxy->ReceiveIbMessage(from, msg);
    }
}

void EthControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const EthStatus& msg)
{
    if (IsLinkSimulated() && AllowForwardToProxy(from))
    {
        _ethControllerProxy->ReceiveIbMessage(from, msg);
    }
}

void EthControllerFacade::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    _ethController->SetEndpointAddress(endpointAddress);
    _ethControllerProxy->SetEndpointAddress(endpointAddress);
}

auto EthControllerFacade::EndpointAddress() const -> const mw::EndpointAddress&
{
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

auto EthControllerFacade::AllowForwardToDefault(const IIbServiceEndpoint* from) const -> bool
{
    const auto& fromDescr = from->GetServiceDescriptor();
    return fromDescr.participantName != _serviceDescriptor.participantName;
}

auto EthControllerFacade::AllowForwardToProxy(const IIbServiceEndpoint* from) const -> bool
{
    const auto& fromDescr = from->GetServiceDescriptor();
    return _remoteBusSimulator.participantName == fromDescr.participantName &&
           _serviceDescriptor.serviceId == fromDescr.serviceId;
}

auto EthControllerFacade::IsLinkSimulated() const -> bool
{
    return _remoteBusSimulator.isLinkSimulated;
}

} // namespace eth
} // namespace sim
} // namespace ib
