// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "FlexrayControllerFacade.hpp"
#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"

namespace ib {
namespace sim {
namespace fr {

FlexrayControllerFacade::FlexrayControllerFacade(mw::IParticipantInternal* participant, cfg::FlexrayController config,
                                                 mw::sync::ITimeProvider* /*timeProvider*/)
    : _participant{participant}
    , _config{std::move(config)}
{
    _flexrayControllerProxy = std::make_unique<FlexrayControllerProxy>(participant, config, this);
    _currentController = _flexrayControllerProxy.get();
}

void FlexrayControllerFacade::Configure(const FlexrayControllerConfig& config)
{
    _currentController->Configure(config);
}

void FlexrayControllerFacade::ReconfigureTxBuffer(uint16_t txBufferIdx, const FlexrayTxBufferConfig& config)
{
    _currentController->ReconfigureTxBuffer(std::move(txBufferIdx), config);
}

void FlexrayControllerFacade::UpdateTxBuffer(const FlexrayTxBufferUpdate& update)
{
    _currentController->UpdateTxBuffer(update);
}

void FlexrayControllerFacade::Run()
{
    _currentController->Run();
}

void FlexrayControllerFacade::DeferredHalt()
{
    _currentController->DeferredHalt();
}

void FlexrayControllerFacade::Freeze()
{
    _currentController->Freeze();
}

void FlexrayControllerFacade::AllowColdstart()
{
    _currentController->AllowColdstart();
}

void FlexrayControllerFacade::AllSlots()
{
    _currentController->AllSlots();
}

void FlexrayControllerFacade::Wakeup()
{
    _currentController->Wakeup();
}

void FlexrayControllerFacade::AddFrameHandler(FrameHandler handler)
{
    _flexrayControllerProxy->AddFrameHandler(std::move(handler));
}

void FlexrayControllerFacade::AddFrameTransmitHandler(FrameTransmitHandler handler)
{
    _flexrayControllerProxy->AddFrameTransmitHandler(std::move(handler));
}

void FlexrayControllerFacade::AddWakeupHandler(WakeupHandler handler)
{
    _flexrayControllerProxy->AddWakeupHandler(std::move(handler));
}

void FlexrayControllerFacade::AddPocStatusHandler(PocStatusHandler handler)
{
    _flexrayControllerProxy->AddPocStatusHandler(std::move(handler));
}

void FlexrayControllerFacade::AddSymbolHandler(SymbolHandler handler)
{
    _flexrayControllerProxy->AddSymbolHandler(std::move(handler));
}

void FlexrayControllerFacade::AddSymbolTransmitHandler(SymbolTransmitHandler handler)
{
    _flexrayControllerProxy->AddSymbolTransmitHandler(std::move(handler));
}

void FlexrayControllerFacade::AddCycleStartHandler(CycleStartHandler handler)
{
    _flexrayControllerProxy->AddCycleStartHandler(std::move(handler));
}

// IIbToFlexrayControllerFacade
void FlexrayControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexrayFrameEvent& msg)
{
    if (IsNetworkSimulated())
    {
        if (AllowForwardToProxy(from))
            _flexrayControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        throw std::logic_error("This controller mode is not supported anymore");
    }
}

void FlexrayControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexrayFrameTransmitEvent& msg)
{
    if (IsNetworkSimulated())
    {
        if (AllowForwardToProxy(from))
            _flexrayControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        throw std::logic_error("This controller mode is not supported anymore");
    }
}

void FlexrayControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexraySymbolEvent& msg)
{
    if (IsNetworkSimulated())
    {
        if (AllowForwardToProxy(from))
            _flexrayControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        throw std::logic_error("This controller mode is not supported anymore");
    }
}

void FlexrayControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexraySymbolTransmitEvent& msg)
{
    if (IsNetworkSimulated())
    {
        if (AllowForwardToProxy(from))
            _flexrayControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        throw std::logic_error("This controller mode is not supported anymore");
    }
}

void FlexrayControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexrayCycleStartEvent& msg)
{
    if (IsNetworkSimulated() && AllowForwardToProxy(from))
    {
        _flexrayControllerProxy->ReceiveIbMessage(from, msg);
    }
}

void FlexrayControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexrayPocStatusEvent& msg)
{
    if (IsNetworkSimulated() && AllowForwardToProxy(from))
    {
        _flexrayControllerProxy->ReceiveIbMessage(from, msg);
    }
}

// ITraceMessageSource
void FlexrayControllerFacade::AddSink(extensions::ITraceMessageSink* sink)
{
    _flexrayControllerProxy->AddSink(sink);
}

// IIbServiceEndpoint
void FlexrayControllerFacade::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
    _flexrayControllerProxy->SetServiceDescriptor(serviceDescriptor);

    mw::service::IServiceDiscovery* disc = _participant->GetServiceDiscovery();
    disc->RegisterServiceDiscoveryHandler(
        [this, serviceDescriptor](mw::service::ServiceDiscoveryEvent::Type discoveryType,
                                  const mw::ServiceDescriptor& remoteServiceDescriptor) {
            // check if discovered service is a network simulator (if none is known)
            if (!IsNetworkSimulated())
            {
                // check if received descriptor has a matching simulated link
                if (discoveryType == mw::service::ServiceDiscoveryEvent::Type::ServiceCreated
                    && IsRelevantNetwork(remoteServiceDescriptor))
                {
                    _simulatedLinkDetected = true;
                    _simulatedLink = remoteServiceDescriptor;
                    _currentController = _flexrayControllerProxy.get();
                }
            }
        });
}

auto FlexrayControllerFacade::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

auto FlexrayControllerFacade::AllowForwardToProxy(const IIbServiceEndpoint* from) const -> bool
{
    const auto& fromDescr = from->GetServiceDescriptor();
    return _simulatedLink.GetParticipantName() == fromDescr.GetParticipantName()
           && _serviceDescriptor.GetServiceId() == fromDescr.GetServiceId();
}

auto FlexrayControllerFacade::IsNetworkSimulated() const -> bool
{
    return _simulatedLinkDetected;
}

auto FlexrayControllerFacade::IsRelevantNetwork(const mw::ServiceDescriptor& remoteServiceDescriptor) const -> bool
{
    return remoteServiceDescriptor.GetServiceType() == ib::mw::ServiceType::Link
           && remoteServiceDescriptor.GetNetworkName() == _serviceDescriptor.GetNetworkName();
}

} // namespace fr
} // namespace sim
} // namespace ib
