// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinControllerFacade.hpp"
#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"

namespace ib {
namespace sim {
namespace lin {

LinControllerFacade::LinControllerFacade(mw::IComAdapterInternal* comAdapter, cfg::LinController config,
                                         mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _config{config}
{
    _linController = std::make_unique<LinController>(comAdapter, timeProvider);
    _linControllerProxy = std::make_unique<LinControllerProxy>(comAdapter);
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

void LinControllerFacade::SendFrame(Frame frame, FrameResponseType responseType)
{
    _currentController->SendFrame(std::move(frame), std::move(responseType));
}

void LinControllerFacade::SendFrame(Frame frame, FrameResponseType responseType, std::chrono::nanoseconds timestamp)
{
    _currentController->SendFrame(std::move(frame), std::move(responseType), std::move(timestamp));
}

void LinControllerFacade::SendFrameHeader(LinIdT linId)
{
    _currentController->SendFrameHeader(std::move(linId));
}

void LinControllerFacade::SendFrameHeader(LinIdT linId, std::chrono::nanoseconds timestamp)
{
    _currentController->SendFrameHeader(std::move(linId), std::move(timestamp));
}

void LinControllerFacade::SetFrameResponse(Frame frame, FrameResponseMode mode)
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

void LinControllerFacade::RegisterFrameStatusHandler(FrameStatusHandler handler)
{
    _linController->RegisterFrameStatusHandler(handler);
    _linControllerProxy->RegisterFrameStatusHandler(handler);
}

void LinControllerFacade::RegisterGoToSleepHandler(GoToSleepHandler handler)
{
    _linController->RegisterGoToSleepHandler(handler);
    _linControllerProxy->RegisterGoToSleepHandler(handler);
}

void LinControllerFacade::RegisterWakeupHandler(WakeupHandler handler)
{
    _linController->RegisterWakeupHandler(handler);
    _linControllerProxy->RegisterWakeupHandler(handler);
}

void LinControllerFacade::RegisterFrameResponseUpdateHandler(FrameResponseUpdateHandler handler)
{
    _linController->RegisterFrameResponseUpdateHandler(handler);
    _linControllerProxy->RegisterFrameResponseUpdateHandler(handler);
}

// IIbToLinController
void LinControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const Transmission& msg)
{
    if (IsLinkSimulated())
    {
        if (ProxyFilter(from)) _linControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        _linController->ReceiveIbMessage(from, msg);
    }
}

void LinControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const WakeupPulse& msg)
{
    if (IsLinkSimulated())
    {
        if (ProxyFilter(from)) _linControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        _linController->ReceiveIbMessage(from, msg);
    }
}

void LinControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const ControllerConfig& msg)
{
    if (IsLinkSimulated())
    {
        if (ProxyFilter(from)) _linControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        _linController->ReceiveIbMessage(from, msg);
    }
}

void LinControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const ControllerStatusUpdate& msg)
{
    if (!IsLinkSimulated())
    {
        _linController->ReceiveIbMessage(from, msg);
    }
}

void LinControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const FrameResponseUpdate& msg)
{
    if (IsLinkSimulated())
    {
        if (ProxyFilter(from)) _linControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        _linController->ReceiveIbMessage(from, msg);
    }
}

void LinControllerFacade::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    // TODO remove support
    _linController->SetEndpointAddress(endpointAddress);
    _linControllerProxy->SetEndpointAddress(endpointAddress);
}

auto LinControllerFacade::EndpointAddress() const -> const mw::EndpointAddress&
{
    // TODO remove!
    if (IsLinkSimulated())
    {
        return _linControllerProxy->EndpointAddress();
    }
    else
    {
        return _linController->EndpointAddress();
    }
}

//ib::mw::sync::ITimeConsumer
void LinControllerFacade::SetTimeProvider(mw::sync::ITimeProvider* timeProvider)
{
    if (!IsLinkSimulated())
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
                    _currentController = _linControllerProxy.get();
                }
            }
            else
            {
                if (discoveryType == mw::service::ServiceDiscoveryEvent::Type::ServiceRemoved
                    && remoteServiceDescriptor.type == serviceDescriptor.type
                    && remoteServiceDescriptor.linkName == serviceDescriptor.linkName)
                {
                    _remoteBusSimulator.isLinkSimulated = false;
                    _currentController = _linController.get();
                }
            }
        });
}

auto LinControllerFacade::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

auto LinControllerFacade::DefaultFilter(const IIbServiceEndpoint* from) const -> bool
{
    return true;
}

auto LinControllerFacade::ProxyFilter(const IIbServiceEndpoint* from) const -> bool
{
    return _remoteBusSimulator.participantName == from->GetServiceDescriptor().participantName;
}

auto LinControllerFacade::IsLinkSimulated() const -> bool
{
    return _remoteBusSimulator.isLinkSimulated;
}

} // namespace lin
} // namespace sim
} // namespace ib
