// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CanControllerFacade.hpp"
#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"

#include <memory>


namespace ib {
namespace sim {
namespace can {

  
CanControllerFacade::CanControllerFacade(mw::IComAdapterInternal* comAdapter, ib::cfg::CanController config,
                                         mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _config{config}
{
    _canController = std::make_unique<CanController>(comAdapter, config, timeProvider);
    _canControllerProxy = std::make_unique<CanControllerProxy>(comAdapter);
    _currentController = _canController.get();
}

// ICanController
void CanControllerFacade::SetBaudRate(uint32_t rate, uint32_t fdRate)
{
    _canController->SetBaudRate(rate, fdRate);
    _canControllerProxy->SetBaudRate(rate, fdRate);
}

void CanControllerFacade::Reset()
{
    _currentController->Reset();
}

void CanControllerFacade::Start()
{
    _currentController->Start();
}

void CanControllerFacade::Stop()
{
    _currentController->Stop();
}

void CanControllerFacade::Sleep()
{
    _currentController->Sleep();
}

auto CanControllerFacade::SendMessage(const CanMessage& msg) ->CanTxId
{
    return _currentController->SendMessage(msg);
}

auto CanControllerFacade::SendMessage(CanMessage&& msg) ->CanTxId
{
    return _currentController->SendMessage(std::move(msg));
}

// IIbToCanController
void CanControllerFacade::RegisterReceiveMessageHandler(ReceiveMessageHandler handler)
{
    _canController->RegisterReceiveMessageHandler(handler);
    _canControllerProxy->RegisterReceiveMessageHandler(std::move(handler));
}

void CanControllerFacade::RegisterStateChangedHandler(StateChangedHandler handler)
{
    _canController->RegisterStateChangedHandler(handler);
    _canControllerProxy->RegisterStateChangedHandler(std::move(handler));
}

void CanControllerFacade::RegisterErrorStateChangedHandler(ErrorStateChangedHandler handler)
{
    _canController->RegisterErrorStateChangedHandler(handler);
    _canControllerProxy->RegisterErrorStateChangedHandler(std::move(handler));
}

void CanControllerFacade::RegisterTransmitStatusHandler(MessageStatusHandler handler)
{
    _canController->RegisterTransmitStatusHandler(handler);
    _canControllerProxy->RegisterTransmitStatusHandler(std::move(handler));
}

// IIbToCanController / IIbToCanControllerProxy
void CanControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const sim::can::CanMessage& msg)
{
    if (IsLinkSimulated())
    {
        if (AllowForwardToProxy(from)) _canControllerProxy->ReceiveIbMessage(from, msg); 
    }
    else
    {
        if (AllowForwardToDefault(from)) _canController->ReceiveIbMessage(from, msg);
    }
}

// IIbToCanControllerProxy only
void CanControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const sim::can::CanControllerStatus& msg)
{
    if (IsLinkSimulated() && AllowForwardToProxy(from))
    {
        _canControllerProxy->ReceiveIbMessage(from, msg);
    }
}

void CanControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const sim::can::CanTransmitAcknowledge& msg)
{
    if (IsLinkSimulated() && AllowForwardToProxy(from))
    {
        _canControllerProxy->ReceiveIbMessage(from, msg);
    }
}


void CanControllerFacade::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    _canController->SetEndpointAddress(endpointAddress);
    _canControllerProxy->SetEndpointAddress(endpointAddress);
}

auto CanControllerFacade::EndpointAddress() const -> const mw::EndpointAddress&
{
    if (IsLinkSimulated())
    {
        return _canControllerProxy->EndpointAddress();
    }
    else
    {
        return _canController->EndpointAddress();
    }
}

void CanControllerFacade::SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider)
{
    if (!IsLinkSimulated())
    {
        _canController->SetTimeProvider(timeProvider);
    }
}

//ITraceMessageSource
void CanControllerFacade::AddSink(extensions::ITraceMessageSink* sink)
{
    _canController->AddSink(sink);
    _canControllerProxy->AddSink(sink);
}

// IIbServiceEndpoint
void CanControllerFacade::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
    _canController->SetServiceDescriptor(serviceDescriptor);
    _canControllerProxy->SetServiceDescriptor(serviceDescriptor);

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
                    _currentController = _canControllerProxy.get();
                }
            }
            else
            {
                if (discoveryType == mw::service::ServiceDiscoveryEvent::Type::ServiceRemoved
                    && remoteServiceDescriptor.type == serviceDescriptor.type
                    && remoteServiceDescriptor.linkName == serviceDescriptor.linkName)
                {
                    _remoteBusSimulator.isLinkSimulated = false;
                    _currentController = _canController.get();
                }
            }
        });
}

auto CanControllerFacade::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

auto CanControllerFacade::AllowForwardToDefault(const IIbServiceEndpoint* from) const -> bool
{
    const auto& fromDescr = from->GetServiceDescriptor();
    return fromDescr.participantName != _serviceDescriptor.participantName;
}

auto CanControllerFacade::AllowForwardToProxy(const IIbServiceEndpoint* from) const
    -> bool
{
    const auto& fromDescr = from->GetServiceDescriptor();
    return _remoteBusSimulator.participantName == fromDescr.participantName &&
           _serviceDescriptor.serviceId == fromDescr.serviceId;
}

auto CanControllerFacade::IsLinkSimulated() const -> bool
{
    return _remoteBusSimulator.isLinkSimulated;
}

} // namespace can
} // namespace sim
} // namespace ib
