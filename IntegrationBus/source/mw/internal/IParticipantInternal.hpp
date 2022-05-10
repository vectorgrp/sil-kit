// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <atomic>

#include "ib/mw/IParticipant.hpp"

// IbInternal component:
#include "internal_fwd.hpp"
#include "IIbServiceEndpoint.hpp"
// IbMwDiscovery
#include "ServiceDatatypes.hpp"

namespace ib {
namespace mw {

class IParticipantInternal : public IParticipant
{
public:
    // ----------------------------------------
    // Public methods
    virtual auto GetParticipantName() const -> const std::string& = 0;
    virtual auto IsSynchronized() const -> bool = 0;

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
    virtual void RegisterCanSimulator(sim::can::IIbToCanSimulator* busSim, const std::vector<std::string>& networkNames) = 0 ;
    virtual void RegisterEthSimulator(sim::eth::IIbToEthSimulator* busSim, const std::vector<std::string>& networkNames) = 0 ;
    virtual void RegisterFlexraySimulator(sim::fr::IIbToFrBusSimulator* busSim, const std::vector<std::string>& networkNames) = 0 ;
    virtual void RegisterLinSimulator(sim::lin::IIbToLinSimulator* busSim, const std::vector<std::string>& networkNames) = 0;

    // The SendIbMessages are virtual functions so we can mock them in testing.
    // For performance reasons this may change in the future.

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::can::CanFrameEvent& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, sim::can::CanFrameEvent&& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::can::CanFrameTransmitEvent& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::can::CanControllerStatus& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::can::CanConfigureBaudrate& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::can::CanSetControllerMode& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::eth::EthernetFrameEvent& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, sim::eth::EthernetFrameEvent&& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::eth::EthernetFrameTransmitEvent& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::eth::EthernetStatus& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::eth::EthernetSetMode& msg) = 0;

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
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::fr::PocStatus& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::lin::LinSendFrameRequest& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::lin::LinSendFrameHeaderRequest& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::lin::LinTransmission& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::lin::LinWakeupPulse& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::lin::LinControllerConfig& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::lin::LinControllerStatusUpdate& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::lin::LinFrameResponseUpdate& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::data::DataMessageEvent& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, sim::data::DataMessageEvent&& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::rpc::FunctionCall& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, sim::rpc::FunctionCall&& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sim::rpc::FunctionCallResponse& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, sim::rpc::FunctionCallResponse&& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sync::NextSimTask& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sync::ParticipantStatus& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sync::ParticipantCommand& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sync::SystemCommand& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const sync::ExpectedParticipants& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const logging::LogMsg& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, logging::LogMsg&& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const service::ParticipantDiscoveryEvent& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const service::ServiceDiscoveryEvent& msg) = 0;

    // targeted messaging
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanFrameEvent& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::can::CanFrameEvent&& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanFrameTransmitEvent& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanControllerStatus& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanConfigureBaudrate& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanSetControllerMode& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::eth::EthernetFrameEvent& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::eth::EthernetFrameEvent&& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::eth::EthernetFrameTransmitEvent& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::eth::EthernetStatus& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::eth::EthernetSetMode& msg) = 0;

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
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::PocStatus& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinSendFrameRequest& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinSendFrameHeaderRequest& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinTransmission& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinWakeupPulse& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinControllerConfig& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinControllerStatusUpdate& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinFrameResponseUpdate& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::data::DataMessageEvent& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::data::DataMessageEvent&& msg) = 0;
    
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::rpc::FunctionCall& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::rpc::FunctionCall&& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::rpc::FunctionCallResponse& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::rpc::FunctionCallResponse&& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sync::NextSimTask& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sync::ParticipantStatus& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sync::ParticipantCommand& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sync::SystemCommand& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const sync::ExpectedParticipants& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const logging::LogMsg& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, logging::LogMsg&& msg) = 0;

    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const service::ParticipantDiscoveryEvent& msg) = 0;
    virtual void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, const std::string& targetParticipantName, const service::ServiceDiscoveryEvent& msg) = 0;

    // For Connection/middleware support:
    virtual void OnAllMessagesDelivered(std::function<void()> callback) = 0;
    virtual void FlushSendBuffers() = 0;
    virtual void ExecuteDeferred(std::function<void()> callback) = 0;

    //Service discovery for dynamic, configuration-less simulations
    virtual auto GetServiceDiscovery() -> service::IServiceDiscovery* = 0;
	
	// Internal DataSubscriber that is only created on a matching data connection
    virtual auto CreateDataSubscriberInternal(
        const std::string& topic, const std::string& linkName,
        const std::string& mediaType,
        const std::map<std::string, std::string>& publisherLabels, sim::data::DataMessageHandlerT callback,
        sim::data::IDataSubscriber* parent) -> sim::data::DataSubscriberInternal*  = 0;

    // Internal Rpc server that is only created on a matching rpc connection
    virtual auto CreateRpcServerInternal(const std::string& rpcChannel, const std::string& linkName,
                                         const std::string& mediaType, const std::map<std::string, std::string>& labels,
                                         sim::rpc::CallProcessor handler, sim::rpc::IRpcServer* parent)
        -> ib::sim::rpc::RpcServerInternal* = 0;

protected:
    std::atomic<EndpointId> _localEndpointId{ 0 };
};

} // mw
} // namespace ib

