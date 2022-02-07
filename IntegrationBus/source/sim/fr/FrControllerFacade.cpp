// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "FrControllerFacade.hpp"
#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"

namespace ib {
namespace sim {
namespace fr {

 FrControllerFacade::FrControllerFacade(mw::IComAdapterInternal* comAdapter, cfg::FlexrayController config,
                                       mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _config{config}
{
    _frController = std::make_unique<FrController>(comAdapter, timeProvider);
    _frControllerProxy = std::make_unique<FrControllerProxy>(comAdapter);
    _currentController = _frController.get();
}

 // CHECK PROXY BEHAVIOR
void FrControllerFacade::Configure(const ControllerConfig& config)
{
    _currentController->Configure(config);
}

void FrControllerFacade::ReconfigureTxBuffer(uint16_t txBufferIdx, const TxBufferConfig& config)
{
    _currentController->ReconfigureTxBuffer(std::move(txBufferIdx), config);
}

void FrControllerFacade::UpdateTxBuffer(const TxBufferUpdate& update)
{
    _currentController->UpdateTxBuffer(update);
}

void FrControllerFacade::Run()
{
    _currentController->Run();
}

void FrControllerFacade::DeferredHalt()
{
    _currentController->DeferredHalt();
}

void FrControllerFacade::Freeze()
{
    _currentController->Freeze();
}

void FrControllerFacade::AllowColdstart()
{
    _currentController->AllowColdstart();
}

void FrControllerFacade::AllSlots()
{
    _currentController->AllSlots();
}

void FrControllerFacade::Wakeup()
{
    _currentController->Wakeup();
}

void FrControllerFacade::RegisterMessageHandler(MessageHandler handler)
{
    _frController->RegisterMessageHandler(handler);
    _frControllerProxy->RegisterMessageHandler(std::move(handler));
}

void FrControllerFacade::RegisterMessageAckHandler(MessageAckHandler handler)
{
    _frController->RegisterMessageAckHandler(handler);
    _frControllerProxy->RegisterMessageAckHandler(std::move(handler));
}

void FrControllerFacade::RegisterWakeupHandler(WakeupHandler handler)
{
    _frController->RegisterWakeupHandler(handler);
    _frControllerProxy->RegisterWakeupHandler(std::move(handler));
}

void FrControllerFacade::RegisterControllerStatusHandler(ControllerStatusHandler handler)
{
    _frController->RegisterControllerStatusHandler(handler);
    _frControllerProxy->RegisterControllerStatusHandler(std::move(handler));
}

void FrControllerFacade::RegisterPocStatusHandler(PocStatusHandler handler)
{
    _frController->RegisterPocStatusHandler(handler);
    _frControllerProxy->RegisterPocStatusHandler(std::move(handler));
}

void FrControllerFacade::RegisterSymbolHandler(SymbolHandler handler)
{
    _frController->RegisterSymbolHandler(handler);
    _frControllerProxy->RegisterSymbolHandler(std::move(handler));
}

void FrControllerFacade::RegisterSymbolAckHandler(SymbolAckHandler handler)
{
    _frController->RegisterSymbolAckHandler(handler);
    _frControllerProxy->RegisterSymbolAckHandler(std::move(handler));
}

void FrControllerFacade::RegisterCycleStartHandler(CycleStartHandler handler)
{
    _frController->RegisterCycleStartHandler(handler);
    _frControllerProxy->RegisterCycleStartHandler(std::move(handler));
}

// IIbToFrController
void FrControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const FrMessage& msg)
{
    if (IsNetworkSimulated())
    {
        if (AllowForwardToProxy(from)) _frControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        throw std::logic_error("This controller mode is not supported anymore");
    }
}

void FrControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const FrMessageAck& msg)
{
    if (IsNetworkSimulated())
    {
        if (AllowForwardToProxy(from)) _frControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        throw std::logic_error("This controller mode is not supported anymore");
    }
}

void FrControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const FrSymbol& msg)
{
    if (IsNetworkSimulated())
    {
        if (AllowForwardToProxy(from)) _frControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        throw std::logic_error("This controller mode is not supported anymore");
    }
}

void FrControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const FrSymbolAck& msg)
{
    if (IsNetworkSimulated())
    {
        if (AllowForwardToProxy(from)) _frControllerProxy->ReceiveIbMessage(from, msg);
    }
    else
    {
        throw std::logic_error("This controller mode is not supported anymore");
    }
}

void FrControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const CycleStart& msg)
{
    if (IsNetworkSimulated() && AllowForwardToProxy(from))
    {
        _frControllerProxy->ReceiveIbMessage(from, msg);
    }
}

void FrControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const PocStatus& msg)
{
    if (IsNetworkSimulated() && AllowForwardToProxy(from))
    {
        _frControllerProxy->ReceiveIbMessage(from, msg);
    }
}

void FrControllerFacade::SetTimeProvider(mw::sync::ITimeProvider* timeProvider)
{
    if (!IsNetworkSimulated())
    {
        dynamic_cast<FrController*>(_currentController)->SetTimeProvider(timeProvider);
    }
}

// ITraceMessageSource
void FrControllerFacade::AddSink(extensions::ITraceMessageSink* sink)
{
    _frController->AddSink(sink);
    _frControllerProxy->AddSink(sink);
}

// IIbServiceEndpoint
void FrControllerFacade::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
    _frController->SetServiceDescriptor(serviceDescriptor);
    _frControllerProxy->SetServiceDescriptor(serviceDescriptor);

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
                    _currentController = _frControllerProxy.get();
                }
            }
            else
            {
                if (discoveryType == mw::service::ServiceDiscoveryEvent::Type::ServiceRemoved
                    && IsRelevantNetwork(remoteServiceDescriptor))
                {
                    _simulatedLinkDetected = false;
                    _currentController = _frController.get();
                }
            }
        });
}

auto FrControllerFacade::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

auto FrControllerFacade::AllowForwardToDefault(const IIbServiceEndpoint* from) const -> bool
{
    throw std::logic_error("This controller mode is not supported anymore");
}

auto FrControllerFacade::AllowForwardToProxy(const IIbServiceEndpoint* from) const -> bool
{
    const auto& fromDescr = from->GetServiceDescriptor();
    return _simulatedLink.GetParticipantName() == fromDescr.GetParticipantName()
           && _serviceDescriptor.GetServiceId() == fromDescr.GetServiceId();
}

auto FrControllerFacade::IsNetworkSimulated() const -> bool
{
    return _simulatedLinkDetected;
}

auto FrControllerFacade::IsRelevantNetwork(const mw::ServiceDescriptor& remoteServiceDescriptor) const -> bool
{
    return remoteServiceDescriptor.GetServiceType() == ib::mw::ServiceType::Link
           && remoteServiceDescriptor.GetNetworkName() == _serviceDescriptor.GetNetworkName();
}

} // namespace fr
} // namespace sim
} // namespace ib
