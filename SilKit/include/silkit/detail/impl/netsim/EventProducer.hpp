// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/capi/EventProducer.h"
#include "silkit/experimental/netsim/INetworkSimulator.hpp"

#include "silkit/detail/impl/HourglassConversions.hpp"

#include "silkit/capi/InterfaceIdentifiers.h"
#include "silkit/detail/impl/ThrowOnError.hpp"

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Experimental {
namespace NetworkSimulation {

// --------------------------------
// CAN
// --------------------------------

class CanEventProducer : public SilKit::Experimental::NetworkSimulation::Can::ICanEventProducer
{
public:
    inline CanEventProducer(SilKit_Experimental_CanEventProducer* canEventProducer);

    inline ~CanEventProducer() override = default;

    inline void Produce(
        const SilKit::Services::Can::CanFrameEvent& cxxEvent,
        const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers) override;

    inline void Produce(
        const SilKit::Services::Can::CanFrameTransmitEvent& cxxEvent,
        const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers) override;

    inline void Produce(
        const SilKit::Services::Can::CanStateChangeEvent& cxxEvent,
        const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers) override;

    inline void Produce(
        const SilKit::Services::Can::CanErrorStateChangeEvent& cxxEvent,
        const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers) override;

private:
    SilKit_Experimental_CanEventProducer* _canEventProducer{nullptr};
};

// --------------------------------
// FlexRay
// --------------------------------

class FlexRayEventProducer : public SilKit::Experimental::NetworkSimulation::Flexray::IFlexRayEventProducer
{
public:
    inline FlexRayEventProducer(SilKit_Experimental_FlexRayEventProducer* flexRayEventProducer);

    inline ~FlexRayEventProducer() override = default;

    inline void Produce(
        const SilKit::Services::Flexray::FlexrayFrameEvent& cxxEvent,
        const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers) override;

    inline void Produce(
        const SilKit::Services::Flexray::FlexrayFrameTransmitEvent& cxxEvent,
        const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers) override;

    inline void Produce(
        const SilKit::Services::Flexray::FlexraySymbolEvent& cxxEvent,
        const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers) override;

    inline void Produce(
        const SilKit::Services::Flexray::FlexraySymbolTransmitEvent& cxxEvent,
        const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers) override;

    inline void Produce(
        const SilKit::Services::Flexray::FlexrayCycleStartEvent& cxxEvent,
        const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers) override;

    inline void Produce(
        const SilKit::Services::Flexray::FlexrayPocStatusEvent& cxxEvent,
        const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers) override;

private:
    SilKit_Experimental_FlexRayEventProducer* _flexRayEventProducer{nullptr};
};

// --------------------------------
// Ethernet
// --------------------------------

class EthernetEventProducer : public SilKit::Experimental::NetworkSimulation::Ethernet::IEthernetEventProducer
{
public:
    inline EthernetEventProducer(SilKit_Experimental_EthernetEventProducer* ethernetEventProducer);

    inline ~EthernetEventProducer() override = default;

    inline void Produce(
        const SilKit::Services::Ethernet::EthernetFrameEvent& cxxEvent,
        const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers) override;
    inline void Produce(
        const SilKit::Services::Ethernet::EthernetFrameTransmitEvent& cxxEvent,
        const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers) override;
    inline void Produce(
        const SilKit::Services::Ethernet::EthernetStateChangeEvent& cxxEvent,
        const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers) override;
    inline void Produce(
        const SilKit::Services::Ethernet::EthernetBitrateChangeEvent& cxxEvent,
        const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers) override;

private:
    SilKit_Experimental_EthernetEventProducer* _ethernetEventProducer{nullptr};
};

// --------------------------------
// Lin
// --------------------------------

class LinEventProducer : public SilKit::Experimental::NetworkSimulation::Lin::ILinEventProducer
{
public:
    inline LinEventProducer(SilKit_Experimental_LinEventProducer* LinEventProducer);

    inline ~LinEventProducer() override = default;

    inline void Produce(
        const SilKit::Services::Lin::LinFrameStatusEvent& cxxEvent,
        const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers) override;
    inline void Produce(
        const SilKit::Services::Lin::LinSendFrameHeaderRequest& cxxEvent,
        const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers) override;
    inline void Produce(
        const SilKit::Services::Lin::LinWakeupEvent& cxxEvent,
        const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers) override;

private:
    SilKit_Experimental_LinEventProducer* _linEventProducer{nullptr};
};

// --------------------------------
//  Receivers
// --------------------------------

inline SilKit_Experimental_EventReceivers assignReceivers(
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_Experimental_EventReceivers receivers;
    SilKit_Struct_Init(SilKit_Experimental_EventReceivers, receivers);
    receivers.numReceivers = cxxReceivers.size();
    receivers.controllerDescriptors = cxxReceivers.data();
    return receivers;
}

// --------------------------------
// CAN
// --------------------------------

CanEventProducer::CanEventProducer(SilKit_Experimental_CanEventProducer* canEventProducer)
    : _canEventProducer{canEventProducer}
{
}

void CanEventProducer::Produce(
    const SilKit::Services::Can::CanFrameEvent& cxxEvent,
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_CanFrameEvent cEvent;
    SilKit_Struct_Init(SilKit_CanFrameEvent, cEvent);
    SilKit_CanFrame canFrame;
    SilKit_Struct_Init(SilKit_CanFrame, canFrame);
    cEvent.frame = &canFrame;
    assignCxxToC(cxxEvent, cEvent);

    SilKit_Experimental_EventReceivers receivers = assignReceivers(cxxReceivers);

    const auto returnCode = SilKit_Experimental_CanEventProducer_Produce(_canEventProducer, &cEvent.structHeader, &receivers);
    ThrowOnError(returnCode);
}

void CanEventProducer::Produce(
    const SilKit::Services::Can::CanFrameTransmitEvent& cxxEvent,
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_CanFrameTransmitEvent cEvent;
    SilKit_Struct_Init(SilKit_CanFrameTransmitEvent, cEvent);
    assignCxxToC(cxxEvent, cEvent);

    SilKit_Experimental_EventReceivers receivers = assignReceivers(cxxReceivers);

    const auto returnCode = SilKit_Experimental_CanEventProducer_Produce(_canEventProducer, &cEvent.structHeader, &receivers);
    ThrowOnError(returnCode);
}

void CanEventProducer::Produce(
    const SilKit::Services::Can::CanStateChangeEvent& cxxEvent,
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_CanStateChangeEvent cEvent;
    SilKit_Struct_Init(SilKit_CanStateChangeEvent, cEvent);
    assignCxxToC(cxxEvent, cEvent);

    SilKit_Experimental_EventReceivers receivers = assignReceivers(cxxReceivers);

    const auto returnCode = SilKit_Experimental_CanEventProducer_Produce(_canEventProducer, &cEvent.structHeader, &receivers);
    ThrowOnError(returnCode);
}

void CanEventProducer::Produce(
    const SilKit::Services::Can::CanErrorStateChangeEvent& cxxEvent,
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_CanErrorStateChangeEvent cEvent;
    SilKit_Struct_Init(SilKit_CanErrorStateChangeEvent, cEvent);
    assignCxxToC(cxxEvent, cEvent);

    SilKit_Experimental_EventReceivers receivers = assignReceivers(cxxReceivers);

    const auto returnCode = SilKit_Experimental_CanEventProducer_Produce(_canEventProducer, &cEvent.structHeader, &receivers);
    ThrowOnError(returnCode);
}

// --------------------------------
// FlexRay
// --------------------------------

FlexRayEventProducer::FlexRayEventProducer(SilKit_Experimental_FlexRayEventProducer* flexRayEventProducer)
    : _flexRayEventProducer{flexRayEventProducer}
{
}

void FlexRayEventProducer::Produce(
    const SilKit::Services::Flexray::FlexrayFrameEvent& cxxEvent,
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_FlexrayFrameEvent cEvent;
    SilKit_Struct_Init(SilKit_FlexrayFrameEvent, cEvent);
    SilKit_FlexrayFrame frame;
    SilKit_Struct_Init(SilKit_FlexrayFrame, frame);
    SilKit_FlexrayHeader header;
    SilKit_Struct_Init(SilKit_FlexrayHeader, header);
    cEvent.frame = &frame;
    cEvent.frame->header = &header;
    assignCxxToC(cxxEvent, cEvent);

    SilKit_Experimental_EventReceivers receivers = assignReceivers(cxxReceivers);

    const auto returnCode =
        SilKit_Experimental_FlexRayEventProducer_Produce(_flexRayEventProducer, &cEvent.structHeader, &receivers);
    ThrowOnError(returnCode);
}

void FlexRayEventProducer::Produce(
    const SilKit::Services::Flexray::FlexrayFrameTransmitEvent& cxxEvent,
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_FlexrayFrameTransmitEvent cEvent;
    SilKit_Struct_Init(SilKit_FlexrayFrameTransmitEvent, cEvent);
    SilKit_FlexrayFrame frame;
    SilKit_Struct_Init(SilKit_FlexrayFrame, frame);
    SilKit_FlexrayHeader header;
    SilKit_Struct_Init(SilKit_FlexrayHeader, header);
    cEvent.frame = &frame;
    cEvent.frame->header = &header;
    assignCxxToC(cxxEvent, cEvent);

    SilKit_Experimental_EventReceivers receivers = assignReceivers(cxxReceivers);

    const auto returnCode =
        SilKit_Experimental_FlexRayEventProducer_Produce(_flexRayEventProducer, &cEvent.structHeader, &receivers);
    ThrowOnError(returnCode);
}

void FlexRayEventProducer::Produce(
    const SilKit::Services::Flexray::FlexraySymbolEvent& cxxEvent,
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_FlexraySymbolEvent cEvent;
    SilKit_Struct_Init(SilKit_FlexraySymbolEvent, cEvent);
    assignCxxToC(cxxEvent, cEvent);

    SilKit_Experimental_EventReceivers receivers = assignReceivers(cxxReceivers);

    const auto returnCode =
        SilKit_Experimental_FlexRayEventProducer_Produce(_flexRayEventProducer, &cEvent.structHeader, &receivers);
    ThrowOnError(returnCode);
}

void FlexRayEventProducer::Produce(
    const SilKit::Services::Flexray::FlexraySymbolTransmitEvent& cxxEvent,
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_FlexraySymbolTransmitEvent cEvent;
    SilKit_Struct_Init(SilKit_FlexraySymbolTransmitEvent, cEvent);
    assignCxxToC(cxxEvent, cEvent);

    SilKit_Experimental_EventReceivers receivers = assignReceivers(cxxReceivers);

    const auto returnCode =
        SilKit_Experimental_FlexRayEventProducer_Produce(_flexRayEventProducer, &cEvent.structHeader, &receivers);
    ThrowOnError(returnCode);
}

void FlexRayEventProducer::Produce(
    const SilKit::Services::Flexray::FlexrayCycleStartEvent& cxxEvent,
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_FlexrayCycleStartEvent cEvent;
    SilKit_Struct_Init(SilKit_FlexrayCycleStartEvent, cEvent);
    assignCxxToC(cxxEvent, cEvent);

    SilKit_Experimental_EventReceivers receivers = assignReceivers(cxxReceivers);

    const auto returnCode =
        SilKit_Experimental_FlexRayEventProducer_Produce(_flexRayEventProducer, &cEvent.structHeader, &receivers);
    ThrowOnError(returnCode);
}

void FlexRayEventProducer::Produce(
    const SilKit::Services::Flexray::FlexrayPocStatusEvent& cxxEvent,
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_FlexrayPocStatusEvent cEvent;
    SilKit_Struct_Init(SilKit_FlexrayPocStatusEvent, cEvent);
    assignCxxToC(cxxEvent, cEvent);

    SilKit_Experimental_EventReceivers receivers = assignReceivers(cxxReceivers);

    const auto returnCode =
        SilKit_Experimental_FlexRayEventProducer_Produce(_flexRayEventProducer, &cEvent.structHeader, &receivers);
    ThrowOnError(returnCode);
}

// --------------------------------
// Ethernet
// --------------------------------

EthernetEventProducer::EthernetEventProducer(SilKit_Experimental_EthernetEventProducer* ethernetEventProducer)
    : _ethernetEventProducer{ethernetEventProducer}
{
}

void EthernetEventProducer::Produce(
    const SilKit::Services::Ethernet::EthernetFrameEvent& cxxEvent,
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_EthernetFrameEvent cEvent;
    SilKit_Struct_Init(SilKit_EthernetFrameEvent, cEvent);

    SilKit_EthernetFrame ethFrame;
    SilKit_Struct_Init(SilKit_EthernetFrame, ethFrame);
    cEvent.ethernetFrame = &ethFrame;

    assignCxxToC(cxxEvent, cEvent);
    SilKit_Experimental_EventReceivers receivers = assignReceivers(cxxReceivers);

    const auto returnCode =
        SilKit_Experimental_EthernetEventProducer_Produce(_ethernetEventProducer, &cEvent.structHeader, &receivers);
    ThrowOnError(returnCode);
}

void EthernetEventProducer::Produce(
    const SilKit::Services::Ethernet::EthernetFrameTransmitEvent& cxxEvent,
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_EthernetFrameTransmitEvent cEvent;
    SilKit_Struct_Init(SilKit_EthernetFrameTransmitEvent, cEvent);

    assignCxxToC(cxxEvent, cEvent);
    SilKit_Experimental_EventReceivers receivers = assignReceivers(cxxReceivers);

    const auto returnCode =
        SilKit_Experimental_EthernetEventProducer_Produce(_ethernetEventProducer, &cEvent.structHeader, &receivers);
    ThrowOnError(returnCode);
}

void EthernetEventProducer::Produce(
    const SilKit::Services::Ethernet::EthernetStateChangeEvent& cxxEvent,
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_EthernetStateChangeEvent cEvent;
    SilKit_Struct_Init(SilKit_EthernetStateChangeEvent, cEvent);
    assignCxxToC(cxxEvent, cEvent);
    SilKit_Experimental_EventReceivers receivers = assignReceivers(cxxReceivers);

    const auto returnCode =
        SilKit_Experimental_EthernetEventProducer_Produce(_ethernetEventProducer, &cEvent.structHeader, &receivers);
    ThrowOnError(returnCode);
}

void EthernetEventProducer::Produce(
    const SilKit::Services::Ethernet::EthernetBitrateChangeEvent& cxxEvent,
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_EthernetBitrateChangeEvent cEvent;
    SilKit_Struct_Init(SilKit_EthernetBitrateChangeEvent, cEvent);
    assignCxxToC(cxxEvent, cEvent);
    SilKit_Experimental_EventReceivers receivers = assignReceivers(cxxReceivers);

    const auto returnCode =
        SilKit_Experimental_EthernetEventProducer_Produce(_ethernetEventProducer, &cEvent.structHeader, &receivers);
    ThrowOnError(returnCode);
}

// --------------------------------
// Lin
// --------------------------------

LinEventProducer::LinEventProducer(SilKit_Experimental_LinEventProducer* linEventProducer)
    : _linEventProducer{linEventProducer}
{
}

void LinEventProducer::Produce(
    const SilKit::Services::Lin::LinFrameStatusEvent& cxxEvent,
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_LinFrame cFrame;
    SilKit_Struct_Init(SilKit_LinFrame, cFrame);

    SilKit_LinFrameStatusEvent cEvent;
    SilKit_Struct_Init(SilKit_LinFrameStatusEvent, cEvent);
    cEvent.frame = &cFrame;

    assignCxxToC(cxxEvent, cEvent);
    SilKit_Experimental_EventReceivers receivers = assignReceivers(cxxReceivers);

    const auto returnCode = SilKit_Experimental_LinEventProducer_Produce(_linEventProducer, &cEvent.structHeader, &receivers);
    ThrowOnError(returnCode);
}

void LinEventProducer::Produce(
    const SilKit::Services::Lin::LinSendFrameHeaderRequest& cxxEvent,
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_LinSendFrameHeaderRequest cEvent;
    SilKit_Struct_Init(SilKit_LinSendFrameHeaderRequest, cEvent);
    assignCxxToC(cxxEvent, cEvent);
    SilKit_Experimental_EventReceivers receivers = assignReceivers(cxxReceivers);

    const auto returnCode = SilKit_Experimental_LinEventProducer_Produce(_linEventProducer, &cEvent.structHeader, &receivers);
    ThrowOnError(returnCode);
}

void LinEventProducer::Produce(
    const SilKit::Services::Lin::LinWakeupEvent& cxxEvent,
    const SilKit::Util::Span<const SilKit::Experimental::NetworkSimulation::ControllerDescriptor>& cxxReceivers)
{
    SilKit_LinWakeupEvent cEvent;
    SilKit_Struct_Init(SilKit_LinWakeupEvent, cEvent);
    assignCxxToC(cxxEvent, cEvent);
    SilKit_Experimental_EventReceivers receivers = assignReceivers(cxxReceivers);

    const auto returnCode = SilKit_Experimental_LinEventProducer_Produce(_linEventProducer, &cEvent.structHeader, &receivers);
    ThrowOnError(returnCode);
}

} // namespace NetworkSimulation
} // namespace Experimental
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
