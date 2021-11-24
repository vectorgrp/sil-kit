// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IComAdapter.hpp"
#include "ib/mw/EndpointAddress.hpp"

// IbInternal component:
#include "internal_fwd.hpp"
#include "IIbServiceEndpoint.hpp"

namespace ib {
namespace mw {

class IComAdapterInternal : public IComAdapter
{
public:
    // ----------------------------------------
    // Public methods
    virtual auto GetParticipantName() const -> const std::string & = 0;
    virtual auto GetConfig() const -> const ib::cfg::Config & = 0;

    /*! \brief Join the middleware domain as a participant.
    *
    * Join the middleware domain and become a participant.
    * \param domainId ID of the domain
    *
    * \throw std::exception A participant was created previously, or a
    * participant could not be created.
    */
    virtual void joinIbDomain(uint32_t domainId) = 0;

    // For VIBE-NetworkSimulator integration:
    virtual void RegisterCanSimulator(sim::can::IIbToCanSimulator* busSim) = 0 ;
    virtual void RegisterEthSimulator(sim::eth::IIbToEthSimulator* busSim) = 0 ;
    virtual void RegisterFlexraySimulator(sim::fr::IIbToFrBusSimulator* busSim) = 0 ;
    virtual void RegisterLinSimulator(sim::lin::IIbToLinSimulator* busSim) = 0;

    // The SendIbMessages are virtual functions so we can mock them in testing.
    // For performance reasons this may change in the future.

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::can::CanMessage& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, sim::can::CanMessage&& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::can::CanTransmitAcknowledge& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::can::CanControllerStatus& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::can::CanConfigureBaudrate& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::can::CanSetControllerMode& msg) = 0;
                                
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::eth::EthMessage& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, sim::eth::EthMessage&& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::eth::EthTransmitAcknowledge& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::eth::EthStatus& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::eth::EthSetMode& msg) = 0;
                                
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::fr::FrMessage& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, sim::fr::FrMessage&& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::fr::FrMessageAck& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, sim::fr::FrMessageAck&& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::fr::FrSymbol& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::fr::FrSymbolAck& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::fr::CycleStart& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::fr::HostCommand& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::fr::ControllerConfig& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::fr::TxBufferConfigUpdate& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::fr::TxBufferUpdate& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::fr::ControllerStatus& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::fr::PocStatus& msg) = 0;
                                
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::lin::SendFrameRequest& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::lin::SendFrameHeaderRequest& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::lin::Transmission& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::lin::WakeupPulse& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::lin::ControllerConfig& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::lin::ControllerStatusUpdate& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::lin::FrameResponseUpdate& msg) = 0;
                                
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::io::AnalogIoMessage& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::io::DigitalIoMessage& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::io::PatternIoMessage& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, sim::io::PatternIoMessage&& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::io::PwmIoMessage& msg) = 0;
                                
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::generic::GenericMessage& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, sim::generic::GenericMessage&& msg) = 0;
                                
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint*, const sync::NextSimTask& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint*, const sync::Tick& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint*, const sync::TickDone& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint*, const sync::QuantumRequest& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint*, const sync::QuantumGrant& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint*, const sync::ParticipantStatus& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint*, const sync::ParticipantCommand& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint*, const sync::SystemCommand& msg) = 0;
                                
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint*, const logging::LogMsg& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint*, logging::LogMsg&& msg) = 0;

    // For Connection/Middleware support:
    virtual void OnAllMessagesDelivered(std::function<void(void)> callback) = 0;
    virtual void FlushSendBuffers() = 0;
};

} // mw
} // namespace ib

