// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IComAdapter.hpp"

// IbInternal component:
#include "internal_fwd.hpp"
#include "IIbServiceEndpoint.hpp"
// IbMwDiscovery
#include "ServiceDatatypes.hpp"

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

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sync::NextSimTask& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sync::ParticipantStatus& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sync::ParticipantCommand& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sync::SystemCommand& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const logging::LogMsg& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, logging::LogMsg&& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const service::ServiceAnnouncement& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const service::ServiceDiscoveryEvent& msg) = 0;

    // targeted messaging
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanMessage& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::can::CanMessage&& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanTransmitAcknowledge& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanControllerStatus& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanConfigureBaudrate& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanSetControllerMode& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::eth::EthMessage& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::eth::EthMessage&& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::eth::EthTransmitAcknowledge& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::eth::EthStatus& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::eth::EthSetMode& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FrMessage& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::fr::FrMessage&& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FrMessageAck& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::fr::FrMessageAck&& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FrSymbol& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FrSymbolAck& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::CycleStart& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::HostCommand& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::ControllerConfig& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::TxBufferConfigUpdate& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::TxBufferUpdate& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::ControllerStatus& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::PocStatus& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::SendFrameRequest& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::SendFrameHeaderRequest& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::Transmission& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::WakeupPulse& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::ControllerConfig& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::ControllerStatusUpdate& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::FrameResponseUpdate& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::io::AnalogIoMessage& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::io::DigitalIoMessage& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::io::PatternIoMessage& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::io::PatternIoMessage&& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::io::PwmIoMessage& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::generic::GenericMessage& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::generic::GenericMessage&& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sync::NextSimTask& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sync::ParticipantStatus& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sync::ParticipantCommand& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sync::SystemCommand& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const logging::LogMsg& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, logging::LogMsg&& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const service::ServiceAnnouncement& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const service::ServiceDiscoveryEvent& msg) = 0;

    // For Connection/Middleware support:
    virtual void OnAllMessagesDelivered(std::function<void(void)> callback) = 0;
    virtual void FlushSendBuffers() = 0;
    
    //Service discovery for dynamic, configuration-less simulations
    virtual auto GetServiceDiscovery() -> service::IServiceDiscovery* = 0;
protected:
    EndpointId _localEndpointId = 0;
};

} // mw
} // namespace ib

