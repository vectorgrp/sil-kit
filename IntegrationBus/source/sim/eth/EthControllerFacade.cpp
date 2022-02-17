// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthControllerFacade.hpp"
#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"

namespace ib {
namespace sim {
namespace eth {

EthControllerFacade::EthControllerFacade(mw::IComAdapterInternal* comAdapter,
                                         cfg::v1::datatypes::EthernetController config,
                                         mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _config{config}
{
    _ethController = std::make_unique<EthController>(comAdapter, config, timeProvider, this);
    _ethControllerProxy = std::make_unique<EthControllerProxy>(comAdapter, config, this);
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

// Provided for testing purposes only
auto EthControllerFacade::SendMessage(EthMessage msg) -> EthTxId
{
    if (IsNetworkSimulated())
    {
        return _ethControllerProxy->SendMessage(std::move(msg));
    }
    else
    {
        return _ethController->SendMessage(std::move(msg));
    }
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
    if (IsNetworkSimulated())
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
    if (IsNetworkSimulated() && AllowForwardToProxy(from))
    {
        _ethControllerProxy->ReceiveIbMessage(from, msg);
    }
}

void EthControllerFacade::ReceiveIbMessage(const IIbServiceEndpoint* from, const EthStatus& msg)
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
