// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IParticipantInternal.hpp"

#include <memory>
#include <vector>
#include <unordered_map>
#include <map>
#include <tuple>

#include "ib/mw/all.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/logging/ILogger.hpp"

#include "ParticipantConfiguration.hpp"

// Interfaces relying on IbInternal
#include "IIbToLogMsgSender.hpp"
#include "IIbToLogMsgReceiver.hpp"

#include "IIbToCanSimulator.hpp"
#include "IIbToCanController.hpp"

#include "IIbToEthSimulator.hpp"
#include "IIbToEthController.hpp"

#include "IIbToLinSimulator.hpp"
#include "IIbToLinController.hpp"

#include "IIbToFlexrayBusSimulator.hpp"
#include "IIbToFlexrayController.hpp"

#include "IIbToDataPublisher.hpp"
#include "IIbToDataSubscriber.hpp"
#include "IIbToDataSubscriberInternal.hpp"

#include "IIbToRpcServer.hpp"
#include "IIbToRpcServerInternal.hpp"
#include "IIbToRpcClient.hpp"

#include "IIbToSystemMonitor.hpp"
#include "IIbToSystemController.hpp"
#include "IIbToParticipantController.hpp"
#include "IIbToLifecycleService.hpp"
#include "IIbToTimeSyncService.hpp"

#include "ITraceMessageSink.hpp"
#include "ITraceMessageSource.hpp"

// IbMwService
#include "ServiceDiscovery.hpp"

#include "ProtocolVersion.hpp"

// Add connection types here and make sure they are instantiated in Participant.cpp
#if defined(IB_MW_HAVE_VASIO)
#   include "VAsioConnection.hpp"
#endif

#include "ProtocolVersion.hpp"

namespace ib {
namespace mw {

template <class IbConnectionT>
class Participant : public IParticipantInternal
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    Participant() = default;
    Participant(const Participant&) = default;
    Participant(Participant&&) = default;
    Participant(cfg::ParticipantConfiguration participantConfig,
               const std::string& participantName, bool isSynchronized,
        ProtocolVersion version = CurrentProtocolVersion());

public:
    // ----------------------------------------
    // Operator Implementations
    Participant& operator=(Participant& other) = default;
    Participant& operator=(Participant&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IParticipant
    auto CreateCanController(const std::string& canonicalName, const std::string& networkName)
        -> sim::can::ICanController* override;
    auto CreateCanController(const std::string& canonicalName) -> sim::can::ICanController* override;
    auto CreateEthernetController(const std::string& canonicalName, const std::string& networkName)
        -> sim::eth::IEthernetController* override;
    auto CreateEthernetController(const std::string& canonicalName) -> sim::eth::IEthernetController* override;
    auto CreateFlexrayController(const std::string& canonicalName, const std::string& networkName)
        -> sim::fr::IFlexrayController* override;
    auto CreateFlexrayController(const std::string& canonicalName) -> sim::fr::IFlexrayController* override;
    auto CreateLinController(const std::string& canonicalName, const std::string& networkName)
        -> sim::lin::ILinController* override;
    auto CreateLinController(const std::string& canonicalName) -> sim::lin::ILinController* override;

    auto CreateDataPublisher(const std::string& canonicalName, const std::string& topic,
                             const std::string& mediaType,
                             const std::map<std::string, std::string>& labels, size_t history = 0)
        -> sim::data::IDataPublisher* override;
    auto CreateDataPublisher(const std::string& canonicalName) -> sim::data::IDataPublisher* override;

    auto CreateDataSubscriber(const std::string& canonicalName, const std::string& topic,
                              const std::string& mediaType,
                              const std::map<std::string, std::string>& labels,
                              sim::data::DataMessageHandlerT defaultDataHandler,
                              sim::data::NewDataPublisherHandlerT newDataSourceHandler = nullptr)
        -> sim::data::IDataSubscriber* override;
    auto CreateDataSubscriber(const std::string& canonicalName) -> sim::data::IDataSubscriber* override;

    auto CreateDataSubscriberInternal(const std::string& canonicalName, const std::string& linkName,
                                      const std::string& mediaType,
                                      const std::map<std::string, std::string>& publisherLabels,
                                      sim::data::DataMessageHandlerT callback, sim::data::IDataSubscriber* parent)
        -> sim::data::DataSubscriberInternal* override;

    auto CreateRpcClient(const std::string& canonicalName, const std::string& functionName, const std::string& mediaType,
                         const std::map<std::string, std::string>& labels, sim::rpc::RpcCallResultHandler handler)
        -> sim::rpc::IRpcClient* override;
    auto CreateRpcClient(const std::string& canonicalName) -> sim::rpc::IRpcClient* override;

    auto CreateRpcServer(const std::string& canonicalName, const std::string& functionName, const std::string& mediaType,
                         const std::map<std::string, std::string>& labels, sim::rpc::RpcCallHandler handler)
        -> sim::rpc::IRpcServer* override;
    auto CreateRpcServer(const std::string& canonicalName) -> sim::rpc::IRpcServer* override;

    auto CreateRpcServerInternal(const std::string& functionName, const std::string& linkName,
                                 const std::string& mediaType, const std::map<std::string, std::string>& labels,
                                 sim::rpc::RpcCallHandler handler, sim::rpc::IRpcServer* parent)
        -> sim::rpc::RpcServerInternal* override;

    void DiscoverRpcServers(const std::string& functionName, const std::string& mediaType,
                            const std::map<std::string, std::string>& labels,
                            sim::rpc::RpcDiscoveryResultHandler handler) override;

    auto GetParticipantController() -> sync::IParticipantController* override;
    auto GetLifecycleService() -> sync::ILifecycleService* override;
    auto CreateTimeSyncService(sync::LifecycleService* service) -> sync::TimeSyncService* override;
    auto GetSystemMonitor() -> sync::ISystemMonitor* override;
    auto GetSystemController() -> sync::ISystemController* override;
    auto GetServiceDiscovery() -> service::IServiceDiscovery* override;
    auto GetLogger() -> logging::ILogger* override;
    auto GetParticipantName() const -> const std::string& override { return _participantName; }
    auto IsSynchronized() const -> bool override { return _isSynchronized; }

    void RegisterCanSimulator(sim::can::IIbToCanSimulator* busSim, const std::vector<std::string>& networkNames) override;
    void RegisterEthSimulator(sim::eth::IIbToEthSimulator* busSim, const std::vector<std::string>& networkNames) override;
    void RegisterFlexraySimulator(sim::fr::IIbToFlexrayBusSimulator* busSim, const std::vector<std::string>& networkNames) override;
    void RegisterLinSimulator(sim::lin::IIbToLinSimulator* busSim, const std::vector<std::string>& networkNames) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const sim::can::CanFrameEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, sim::can::CanFrameEvent&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::can::CanFrameTransmitEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::can::CanControllerStatus& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::can::CanConfigureBaudrate& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::can::CanSetControllerMode& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const sim::eth::EthernetFrameEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, sim::eth::EthernetFrameEvent&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::eth::EthernetFrameTransmitEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::eth::EthernetStatus& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::eth::EthernetSetMode& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexrayFrameEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, sim::fr::FlexrayFrameEvent&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexrayFrameTransmitEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, sim::fr::FlexrayFrameTransmitEvent&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexraySymbolEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexraySymbolTransmitEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexrayCycleStartEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexrayHostCommand& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexrayControllerConfig& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexrayTxBufferConfigUpdate& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexrayTxBufferUpdate& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::fr::FlexrayPocStatusEvent& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::LinSendFrameRequest& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::LinSendFrameHeaderRequest& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::LinTransmission& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::LinWakeupPulse& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::LinControllerConfig& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::LinControllerStatusUpdate& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::lin::LinFrameResponseUpdate& msg) override;

    void SendIbMessage(const IIbServiceEndpoint*, const sync::NextSimTask& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const sync::ParticipantStatus& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const sync::ParticipantCommand& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const sync::SystemCommand& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const sync::ExpectedParticipants& msg) override;

    void SendIbMessage(const IIbServiceEndpoint*, const logging::LogMsg& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, logging::LogMsg&& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const sim::data::DataMessageEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, sim::data::DataMessageEvent&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::rpc::FunctionCall& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, sim::rpc::FunctionCall&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const sim::rpc::FunctionCallResponse& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, sim::rpc::FunctionCallResponse&& msg) override;

    void SendIbMessage(const IIbServiceEndpoint*, const service::ParticipantDiscoveryEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const service::ServiceDiscoveryEvent& msg) override;

    // targeted messaging
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanFrameEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::can::CanFrameEvent&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanFrameTransmitEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanControllerStatus& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanConfigureBaudrate& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::can::CanSetControllerMode& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::eth::EthernetFrameEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::eth::EthernetFrameEvent&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::eth::EthernetFrameTransmitEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::eth::EthernetStatus& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::eth::EthernetSetMode& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexrayFrameEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::fr::FlexrayFrameEvent&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexrayFrameTransmitEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::fr::FlexrayFrameTransmitEvent&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexraySymbolEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexraySymbolTransmitEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexrayCycleStartEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexrayHostCommand& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexrayControllerConfig& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexrayTxBufferConfigUpdate& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexrayTxBufferUpdate& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::fr::FlexrayPocStatusEvent& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinSendFrameRequest& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinSendFrameHeaderRequest& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinTransmission& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinWakeupPulse& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinControllerConfig& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinControllerStatusUpdate& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::lin::LinFrameResponseUpdate& msg) override;

    void SendIbMessage(const IIbServiceEndpoint*, const std::string& targetParticipantName, const sync::NextSimTask& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const std::string& targetParticipantName, const sync::ParticipantStatus& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const std::string& targetParticipantName, const sync::ParticipantCommand& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const std::string& targetParticipantName, const sync::SystemCommand& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const std::string& targetParticipantName, const sync::ExpectedParticipants& msg) override;

    void SendIbMessage(const IIbServiceEndpoint*, const std::string& targetParticipantName, const logging::LogMsg& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const std::string& targetParticipantName, logging::LogMsg&& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::data::DataMessageEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::data::DataMessageEvent&& msg) override;

    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::rpc::FunctionCall& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::rpc::FunctionCall&& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, const sim::rpc::FunctionCallResponse& msg) override;
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, sim::rpc::FunctionCallResponse&& msg) override;

    void SendIbMessage(const IIbServiceEndpoint*, const std::string& targetParticipantName, const service::ParticipantDiscoveryEvent& msg) override;
    void SendIbMessage(const IIbServiceEndpoint*, const std::string& targetParticipantName, const service::ServiceDiscoveryEvent& msg) override;

    void OnAllMessagesDelivered(std::function<void()> callback) override;
    void FlushSendBuffers() override;
    void ExecuteDeferred(std::function<void()> callback) override;

public:
    // ----------------------------------------
    // Public methods

    /*! \brief Join the middleware domain as a participant.
    *
    * Join the middleware domain and become a participant.
    * \param domainId ID of the domain
    *
    * \throw std::exception A participant was created previously, or a
    * participant could not be created.
    */
    void joinIbDomain(uint32_t domainId) override;

    // For Testing Purposes:
    inline auto GetIbConnection() -> IbConnectionT& { return _ibConnection; }

private:
    // ----------------------------------------
    // private datatypes
    template<typename ControllerT>
    using ControllerMap = std::unordered_map<std::string, std::unique_ptr<ControllerT>>;

private:
    // ----------------------------------------
    // private methods

    //!< Search for the controller configuration by name and set configured values. Initialize with controller name if no config is found.
    template <typename ConfigT>
    auto GetConfigByControllerName(const std::vector<ConfigT>& controllers, const std::string& canonicalName)
        -> ConfigT;

    //!< Update the controller configuration for a given optional field. Prefers configured values over programmatically passed values.
    template <typename ValueT>
    void UpdateOptionalConfigValue(const std::string& canonicalName, ib::util::Optional<ValueT>& configuredValue,
                                   const ValueT& passedValue);

    template <typename ValueT>
    void LogMismatchBetweenConfigAndPassedValue(const std::string& controllerName, const ValueT& passedValue,
                                                const ValueT& configuredValue);

    void onIbDomainJoined();

    void SetupRemoteLogging();

    void SetTimeProvider(sync::ITimeProvider*);

    template<class IbMessageT>
    void SendIbMessageImpl(const IIbServiceEndpoint* from, IbMessageT&& msg);
    template<class IbMessageT>
    void SendIbMessageImpl(const IIbServiceEndpoint* from, const std::string& targetParticipantName, IbMessageT&& msg);

    template<class ControllerT>
    auto GetController(const std::string& networkName, const std::string& serviceName) -> ControllerT*;

    //!< internal services don't have a link config
    template<class ControllerT, typename... Arg>
    auto CreateInternalController(const std::string& serviceName, const mw::ServiceType serviceType,
                          const mw::SupplementalData& supplementalData, bool publishService, Arg&&... arg) -> ControllerT*;

    //!< Internal controller creation, explicit network argument for ConfigT without network
    template <class ConfigT, class ControllerT, typename... Arg>
    auto CreateController(const ConfigT& config, const std::string& network, const mw::ServiceType serviceType,
                          const mw::SupplementalData& supplementalData, bool publishService, Arg&&... arg)
        -> ControllerT*;

    //!< Internal controller creation, expects config.network
    template <class ConfigT, class ControllerT, typename... Arg>
    auto CreateController(const ConfigT& config, const mw::ServiceType serviceType,
                          const mw::SupplementalData& supplementalData, bool publishService, Arg&&... arg)
        -> ControllerT*;


    template<class IIbToSimulatorT>
    void RegisterSimulator(IIbToSimulatorT* busSim, cfg::NetworkType linkType, const std::vector<std::string>& simulatedNetworkNames);

    template<class ConfigT>
    void AddTraceSinksToSource(extensions::ITraceMessageSource* controller, ConfigT config);

private:
    // ----------------------------------------
    // private members
    std::string _participantName;
    bool _isSynchronized{ false };
    const ib::cfg::ParticipantConfiguration _participantConfig;
    ParticipantId _participantId{0};

    std::shared_ptr<sync::ITimeProvider> _timeProvider{nullptr};

    std::unique_ptr<logging::ILogger> _logger;
    std::vector<std::unique_ptr<extensions::ITraceMessageSink>> _traceSinks;
    //std::unique_ptr<tracing::ReplayScheduler> _replayScheduler;

    std::tuple<
        ControllerMap<sim::can::IIbToCanController>,
        ControllerMap<sim::eth::IIbToEthController>,
        ControllerMap<sim::fr::IIbToFlexrayController>,
        ControllerMap<sim::lin::IIbToLinController>,
        ControllerMap<sim::data::IIbToDataPublisher>,
        ControllerMap<sim::data::IIbToDataSubscriber>,
        ControllerMap<sim::data::IIbToDataSubscriberInternal>,
        ControllerMap<sim::rpc::IIbToRpcClient>,
        ControllerMap<sim::rpc::IIbToRpcServer>,
        ControllerMap<sim::rpc::IIbToRpcServerInternal>,
        ControllerMap<logging::IIbToLogMsgSender>,
        ControllerMap<logging::IIbToLogMsgReceiver>,
        ControllerMap<sync::IIbToLifecycleService>,
        ControllerMap<sync::IIbToParticipantController>,
        ControllerMap<sync::IIbToSystemMonitor>,
        ControllerMap<sync::IIbToSystemController>,
        ControllerMap<sync::IIbToTimeSyncService>,
        ControllerMap<service::ServiceDiscovery>
    > _controllers;

    std::tuple<
        sim::can::IIbToCanSimulator*,
        sim::eth::IIbToEthSimulator*,
        sim::fr::IIbToFlexrayBusSimulator*,
        sim::lin::IIbToLinSimulator*
    > _simulators {nullptr, nullptr, nullptr, nullptr};

    IbConnectionT _ibConnection;

};

} // mw
} // namespace ib

