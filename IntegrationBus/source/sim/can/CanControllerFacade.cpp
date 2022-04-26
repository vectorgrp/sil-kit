// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CanControllerFacade.hpp"
#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"
#include "ParticipantConfiguration.hpp"

#include <memory>


namespace ib {
namespace sim {
namespace can {

  
CanControllerFacade::CanControllerFacade(mw::IParticipantInternal* participant, ib::cfg::CanController config,
                                         mw::sync::ITimeProvider* timeProvider)
    : _participant{participant}
    , _config{config}
{
    _canController = std::make_unique<CanController>(participant, config, timeProvider, this);
    _canControllerProxy = std::make_unique<CanControllerProxy>(participant, this);
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

auto CanControllerFacade::SendFrame(const CanFrame& msg, void* userContext) ->CanTxId
{
    return _currentController->SendFrame(msg, userContext);
}


// IIbToCanController
void CanControllerFacade::AddFrameHandler(FrameHandler handler, DirectionMask directionMask)
{
    _canController->AddFrameHandler(handler, directionMask);
    _canControllerProxy->AddFrameHandler(std::move(handler), directionMask);
}

void CanControllerFacade::AddStateChangeHandler(StateChangeHandler handler)
{
    _canController->AddStateChangeHandler(handler);
    _canControllerProxy->AddStateChangeHandler(std::move(handler));
}

void CanControllerFacade::AddErrorStateChangeHandler(ErrorStateChangeHandler handler)
{
    _canController->AddErrorStateChangeHandler(handler);
    _canControllerProxy->AddErrorStateChangeHandler(std::move(handler));
}

void CanControllerFacade::AddFrameTransmitHandler(FrameTransmitHandler handler, CanTransmitStatusMask statusMask)
{
    _canController->AddFrameTransmitHandler(handler, statusMask);
    _canControllerProxy->AddFrameTransmitHandler(std::move(handler), statusMask);
}

// IIbToCanController / IIbToCanControllerProxy
void CanControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const sim::can::CanFrameEvent& msg)
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

void CanControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const sim::can::CanFrameTransmitEvent& msg)
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
