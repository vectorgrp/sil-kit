// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CanControllerFacade.hpp"
#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"
#include "ParticipantConfiguration.hpp"

#include <memory>


namespace ib {
namespace sim {
namespace can {

  
CanControllerFacade::CanControllerFacade(mw::IComAdapterInternal* comAdapter, ib::cfg::v1::datatypes::CanController config,
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

auto CanControllerFacade::SendMessage(const CanMessage& msg, void* userContext) ->CanTxId
{
    return _currentController->SendMessage(msg, userContext);
}

auto CanControllerFacade::SendMessage(CanMessage&& msg, void* userContext) -> CanTxId
{
    return _currentController->SendMessage(std::move(msg), userContext);
}

// IIbToCanController
void CanControllerFacade::RegisterReceiveMessageHandler(ReceiveMessageHandler handler, DirectionMask directionMask)
{
    _canController->RegisterReceiveMessageHandler(handler, directionMask);
    _canControllerProxy->RegisterReceiveMessageHandler(std::move(handler), directionMask);
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

void CanControllerFacade::RegisterTransmitStatusHandler(MessageStatusHandler handler, CanTransmitStatusMask statusMask)
{
    _canController->RegisterTransmitStatusHandler(handler, statusMask);
    _canControllerProxy->RegisterTransmitStatusHandler(std::move(handler), statusMask);
}

// IIbToCanController / IIbToCanControllerProxy
void CanControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const sim::can::CanMessage& msg)
{
    if (IsNetworkSimulated())
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
    if (IsNetworkSimulated() && AllowForwardToProxy(from))
    {
        _canControllerProxy->ReceiveIbMessage(from, msg);
    }
}

void CanControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const sim::can::CanTransmitAcknowledge& msg)
{
    if (IsNetworkSimulated() && AllowForwardToProxy(from))
    {
        _canControllerProxy->ReceiveIbMessage(from, msg);
    }
}

void CanControllerFacade::SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider)
{
    if (!IsNetworkSimulated())
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
            if (!IsNetworkSimulated())
            {
                // check if received descriptor has a matching simulated link
                if (discoveryType == mw::service::ServiceDiscoveryEvent::Type::ServiceCreated
                    && IsRelevantNetwork(remoteServiceDescriptor))
                {
                    _simulatedLinkDetected = true;
                    _simulatedLink = remoteServiceDescriptor;
                    _currentController = _canControllerProxy.get();
                }
            }
            else
            {
                if (discoveryType == mw::service::ServiceDiscoveryEvent::Type::ServiceRemoved
                    && IsRelevantNetwork(remoteServiceDescriptor))
                {
                    _simulatedLinkDetected = false;
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
    return !(fromDescr.GetParticipantName() == _serviceDescriptor.GetParticipantName()
             && fromDescr.GetServiceId() == _serviceDescriptor.GetServiceId());
}

auto CanControllerFacade::AllowForwardToProxy(const IIbServiceEndpoint* from) const
    -> bool
{
    const auto& fromDescr = from->GetServiceDescriptor();
    return _simulatedLink.GetParticipantName() == fromDescr.GetParticipantName()
           && _serviceDescriptor.GetServiceId() == fromDescr.GetServiceId();
}

auto CanControllerFacade::IsNetworkSimulated() const -> bool
{
    return _simulatedLinkDetected;
}

auto CanControllerFacade::IsRelevantNetwork(const mw::ServiceDescriptor& remoteServiceDescriptor) const -> bool
{
    return remoteServiceDescriptor.GetServiceType() == ib::mw::ServiceType::Link
           && remoteServiceDescriptor.GetNetworkName() == _serviceDescriptor.GetNetworkName();
}

} // namespace can
} // namespace sim
} // namespace ib
