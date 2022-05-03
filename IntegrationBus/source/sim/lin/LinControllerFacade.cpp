// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinControllerFacade.hpp"
#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"

namespace ib {
namespace sim {
namespace lin {

LinControllerFacade::LinControllerFacade(mw::IParticipantInternal* participant, cfg::LinController config,
                                         mw::sync::ITimeProvider* timeProvider)
    : _participant{participant}
    , _config{config}
{
    _linController = std::make_unique<LinController>(participant, timeProvider, this);
    _linControllerProxy = std::make_unique<LinControllerProxy>(participant, this);
    _currentController = _linController.get();
}

void LinControllerFacade::Init(ControllerConfig config)
{
    _currentController->Init(std::move(config));
}

auto LinControllerFacade::Status() const noexcept->ControllerStatus
{
    return _currentController->Status();
}

void LinControllerFacade::SendFrame(LinFrame frame, FrameResponseType responseType)
{
    _currentController->SendFrame(std::move(frame), std::move(responseType));
}

void LinControllerFacade::SendFrameHeader(LinIdT linId)
{
    _currentController->SendFrameHeader(std::move(linId));
}

void LinControllerFacade::SetFrameResponse(LinFrame frame, FrameResponseMode mode)
{
    _currentController->SetFrameResponse(std::move(frame), std::move(mode));
}

void LinControllerFacade::SetFrameResponses(std::vector<FrameResponse> responses)
{
    _currentController->SetFrameResponses(std::move(responses));
}

void LinControllerFacade::GoToSleep()
{
    _currentController->GoToSleep();
}

void LinControllerFacade::GoToSleepInternal()
{
    _currentController->GoToSleepInternal();
}

void LinControllerFacade::Wakeup()
{
    _currentController->Wakeup();
}

void LinControllerFacade::WakeupInternal()
{
    _currentController->WakeupInternal();
}

void LinControllerFacade::AddFrameStatusHandler(FrameStatusHandler handler)
{
    _linController->AddFrameStatusHandler(handler);
    _linControllerProxy->AddFrameStatusHandler(std::move(handler));
}

void LinControllerFacade::AddGoToSleepHandler(GoToSleepHandler handler)
{
    _linController->AddGoToSleepHandler(handler);
    _linControllerProxy->AddGoToSleepHandler(std::move(handler));
}

void LinControllerFacade::AddWakeupHandler(WakeupHandler handler)
{
    _linController->AddWakeupHandler(handler);
    _linControllerProxy->AddWakeupHandler(std::move(handler));
}

void LinControllerFacade::AddFrameResponseUpdateHandler(FrameResponseUpdateHandler handler)
{
    _linController->AddFrameResponseUpdateHandler(handler);
    _linControllerProxy->AddFrameResponseUpdateHandler(std::move(handler));
}

// IIbToLinController
void LinControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const Transmission& msg)
{
    if (IsNetworkSimulated())
    {
        if (AllowForwardToProxy(from)) _linControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        if (AllowForwardToDefault(from)) _linController->ReceiveIbMessage(from, msg);
    }
}

void LinControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const WakeupPulse& msg)
{
    if (IsNetworkSimulated())
    {
        if (AllowForwardToProxy(from)) _linControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        if (AllowForwardToDefault(from)) _linController->ReceiveIbMessage(from, msg);
    }
}

void LinControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const ControllerConfig& msg)
{
    if (IsNetworkSimulated())
    {
        if (AllowForwardToProxy(from)) _linControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        if (AllowForwardToDefault(from)) _linController->ReceiveIbMessage(from, msg);
    }
}

void LinControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const ControllerStatusUpdate& msg)
{
    if (!IsNetworkSimulated())
    {
        if (AllowForwardToDefault(from)) _linController->ReceiveIbMessage(from, msg);
    }
}

void LinControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const FrameResponseUpdate& msg)
{
    if (IsNetworkSimulated())
    {
        if (AllowForwardToProxy(from)) _linControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        if (AllowForwardToDefault(from)) _linController->ReceiveIbMessage(from, msg);
    }
}

//ib::mw::sync::ITimeConsumer
void LinControllerFacade::SetTimeProvider(mw::sync::ITimeProvider* timeProvider)
{
    if (!IsNetworkSimulated())
    {
        _linController->SetTimeProvider(timeProvider);
    }
}

//ITraceMessageSource
void LinControllerFacade::AddSink(extensions::ITraceMessageSink* sink)
{
    _linController->AddSink(sink);
    _linControllerProxy->AddSink(sink);
}

// IIbServiceEndpoint
void LinControllerFacade::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
    _linController->SetServiceDescriptor(serviceDescriptor);
    _linControllerProxy->SetServiceDescriptor(serviceDescriptor);

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
                    _currentController = _linControllerProxy.get();
                }
            }
            else
            {
                if (discoveryType == mw::service::ServiceDiscoveryEvent::Type::ServiceRemoved
                    && IsRelevantNetwork(remoteServiceDescriptor))
                {
                    _simulatedLinkDetected = false;
                    _currentController = _linController.get();
                }
            }
        });
}

auto LinControllerFacade::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

auto LinControllerFacade::AllowForwardToDefault(const IIbServiceEndpoint* from) const -> bool
{
    const auto& fromDescr = from->GetServiceDescriptor();
    return !(fromDescr.GetParticipantName() == _serviceDescriptor.GetParticipantName()
           && fromDescr.GetServiceId() == _serviceDescriptor.GetServiceId());
}

auto LinControllerFacade::AllowForwardToProxy(const IIbServiceEndpoint* from) const -> bool
{
    const auto& fromDescr = from->GetServiceDescriptor();
    return _simulatedLink.GetParticipantName() == fromDescr.GetParticipantName()
           && _serviceDescriptor.GetServiceId() == fromDescr.GetServiceId();
}

auto LinControllerFacade::IsNetworkSimulated() const -> bool
{
    return _simulatedLinkDetected;
}

auto LinControllerFacade::IsRelevantNetwork(const mw::ServiceDescriptor& remoteServiceDescriptor) const -> bool
{
    return remoteServiceDescriptor.GetServiceType() == ib::mw::ServiceType::Link
           && remoteServiceDescriptor.GetNetworkName() == _serviceDescriptor.GetNetworkName();
}

} // namespace lin
} // namespace sim
} // namespace ib
