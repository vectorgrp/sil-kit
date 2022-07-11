// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <atomic>

#include "silkit/participant/IParticipant.hpp"

#include "internal_fwd.hpp"
#include "IServiceEndpoint.hpp"
#include "ServiceDatatypes.hpp"

namespace SilKit {
namespace Core {

class IParticipantInternal : public IParticipant
{
public:
    // ----------------------------------------
    // Public methods
    virtual auto GetParticipantName() const -> const std::string& = 0;

    /*! \brief Join the middleware domain as a participant.
    *
    * Connect to the registry listening on registryUri
    * \param registryUri URI of the registry
    *
    * \throw std::exception A participant was created previously, or a
    * participant could not be created.
    */
    virtual void JoinSilKitDomain(const std::string& registryUri) = 0;

    // For NetworkSimulator integration:
    virtual void RegisterCanSimulator(Services::Can::IMsgForCanSimulator* busSim, const std::vector<std::string>& networkNames) = 0 ;
    virtual void RegisterEthSimulator(Services::Ethernet::IMsgForEthSimulator* busSim, const std::vector<std::string>& networkNames) = 0 ;
    virtual void RegisterFlexraySimulator(Services::Flexray::IMsgForFlexrayBusSimulator* busSim, const std::vector<std::string>& networkNames) = 0 ;
    virtual void RegisterLinSimulator(Services::Lin::IMsgForLinSimulator* busSim, const std::vector<std::string>& networkNames) = 0;

    // The SendMsgs are virtual functions so we can mock them in testing.
    // For performance reasons this may change in the future.

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Can::CanFrameEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, Services::Can::CanFrameEvent&& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Can::CanFrameTransmitEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Can::CanControllerStatus& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Can::CanConfigureBaudrate& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Can::CanSetControllerMode& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Ethernet::EthernetFrameEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, Services::Ethernet::EthernetFrameEvent&& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Ethernet::EthernetFrameTransmitEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Ethernet::EthernetStatus& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Ethernet::EthernetSetMode& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Flexray::FlexrayFrameEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, Services::Flexray::FlexrayFrameEvent&& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Flexray::FlexrayFrameTransmitEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, Services::Flexray::FlexrayFrameTransmitEvent&& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Flexray::FlexraySymbolEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Flexray::FlexraySymbolTransmitEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Flexray::FlexrayCycleStartEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Flexray::FlexrayHostCommand& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Flexray::FlexrayControllerConfig& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Flexray::FlexrayTxBufferConfigUpdate& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Flexray::FlexrayTxBufferUpdate& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Flexray::FlexrayPocStatusEvent& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Lin::LinSendFrameRequest& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Lin::LinSendFrameHeaderRequest& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Lin::LinTransmission& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Lin::LinWakeupPulse& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Lin::LinControllerConfig& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Lin::LinControllerStatusUpdate& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Lin::LinFrameResponseUpdate& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::PubSub::DataMessageEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, Services::PubSub::DataMessageEvent&& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Rpc::FunctionCall& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, Services::Rpc::FunctionCall&& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Rpc::FunctionCallResponse& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, Services::Rpc::FunctionCallResponse&& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Orchestration::NextSimTask& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Orchestration::ParticipantStatus& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Orchestration::ParticipantCommand& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Orchestration::SystemCommand& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Orchestration::WorkflowConfiguration& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Logging::LogMsg& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, Services::Logging::LogMsg&& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Discovery::ParticipantDiscoveryEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Discovery::ServiceDiscoveryEvent& msg) = 0;

    // targeted messaging
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Can::CanFrameEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, Services::Can::CanFrameEvent&& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Can::CanFrameTransmitEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Can::CanControllerStatus& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Can::CanConfigureBaudrate& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Can::CanSetControllerMode& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Ethernet::EthernetFrameEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, Services::Ethernet::EthernetFrameEvent&& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Ethernet::EthernetFrameTransmitEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Ethernet::EthernetStatus& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Ethernet::EthernetSetMode& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::FlexrayFrameEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, Services::Flexray::FlexrayFrameEvent&& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::FlexrayFrameTransmitEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, Services::Flexray::FlexrayFrameTransmitEvent&& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::FlexraySymbolEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::FlexraySymbolTransmitEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::FlexrayCycleStartEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::FlexrayHostCommand& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::FlexrayControllerConfig& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::FlexrayTxBufferConfigUpdate& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::FlexrayTxBufferUpdate& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::FlexrayPocStatusEvent& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Lin::LinSendFrameRequest& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Lin::LinSendFrameHeaderRequest& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Lin::LinTransmission& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Lin::LinWakeupPulse& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Lin::LinControllerConfig& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Lin::LinControllerStatusUpdate& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Lin::LinFrameResponseUpdate& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::PubSub::DataMessageEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, Services::PubSub::DataMessageEvent&& msg) = 0;
    
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Rpc::FunctionCall& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, Services::Rpc::FunctionCall&& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Rpc::FunctionCallResponse& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, Services::Rpc::FunctionCallResponse&& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Orchestration::NextSimTask& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Orchestration::ParticipantStatus& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Orchestration::ParticipantCommand& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Orchestration::SystemCommand& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Orchestration::WorkflowConfiguration& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Logging::LogMsg& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, Services::Logging::LogMsg&& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Discovery::ParticipantDiscoveryEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName, const Discovery::ServiceDiscoveryEvent& msg) = 0;

    // For Connection/middleware support:
    virtual void OnAllMessagesDelivered(std::function<void()> callback) = 0;
    virtual void FlushSendBuffers() = 0;
    virtual void ExecuteDeferred(std::function<void()> callback) = 0;

    //Service discovery for dynamic, configuration-less simulations
    virtual auto GetServiceDiscovery() -> Discovery::IServiceDiscovery* = 0;
	
	// Internal DataSubscriber that is only created on a matching data connection
    virtual auto CreateDataSubscriberInternal(
        const std::string& topic, const std::string& linkName,
        const std::string& mediaType,
        const std::map<std::string, std::string>& publisherLabels, Services::PubSub::DataMessageHandlerT callback,
        Services::PubSub::IDataSubscriber* parent) -> Services::PubSub::DataSubscriberInternal*  = 0;

    // Internal Rpc server that is only created on a matching rpc connection
    virtual auto CreateRpcServerInternal(const std::string& functionName, const std::string& linkName,
                                         const std::string& mediaType, const std::map<std::string, std::string>& labels,
                                         Services::Rpc::RpcCallHandler handler, Services::Rpc::IRpcServer* parent)
        -> SilKit::Services::Rpc::RpcServerInternal* = 0;

    virtual auto CreateTimeSyncService(Services::Orchestration::LifecycleService* service) -> Services::Orchestration::TimeSyncService* = 0;

protected:
    std::atomic<EndpointId> _localEndpointId{ 0 };
};

} // namespace Core
} // namespace SilKit

