// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IParticipantInternal.hpp"

#include <memory>
#include <vector>
#include <unordered_map>
#include <map>
#include <tuple>

#include "silkit/services/all.hpp"
#include "silkit/services/logging/ILogger.hpp"

#include "ParticipantConfiguration.hpp"

// Interfaces relying on SilKitInternal
#include "IMsgForLogMsgSender.hpp"
#include "IMsgForLogMsgReceiver.hpp"

#include "IMsgForCanSimulator.hpp"
#include "IMsgForCanController.hpp"

#include "IMsgForEthSimulator.hpp"
#include "IMsgForEthController.hpp"

#include "IMsgForLinSimulator.hpp"
#include "IMsgForLinController.hpp"

#include "IMsgForFlexrayBusSimulator.hpp"
#include "IMsgForFlexrayController.hpp"

#include "IMsgForDataPublisher.hpp"
#include "IMsgForDataSubscriber.hpp"
#include "IMsgForDataSubscriberInternal.hpp"

#include "IMsgForRpcServer.hpp"
#include "IMsgForRpcServerInternal.hpp"
#include "IMsgForRpcClient.hpp"

#include "IMsgForSystemMonitor.hpp"
#include "IMsgForSystemController.hpp"
#include "IMsgForLifecycleService.hpp"
#include "IMsgForTimeSyncService.hpp"

#include "ITraceMessageSink.hpp"
#include "ITraceMessageSource.hpp"

// SilKitMwService
#include "ServiceDiscovery.hpp"

#include "ProtocolVersion.hpp"
#include "TimeProvider.hpp"

// Add connection types here and make sure they are instantiated in Participant.cpp
#include "VAsioConnection.hpp"

using namespace std::chrono_literals;

namespace SilKit {
namespace Core {

template <class SilKitConnectionT>
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
    Participant(Config::ParticipantConfiguration participantConfig, const std::string& participantName,
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
        -> Services::Can::ICanController* override;
    auto CreateEthernetController(const std::string& canonicalName, const std::string& networkName)
        -> Services::Ethernet::IEthernetController* override;
    auto CreateFlexrayController(const std::string& canonicalName, const std::string& networkName)
        -> Services::Flexray::IFlexrayController* override;
    auto CreateLinController(const std::string& canonicalName, const std::string& networkName)
        -> Services::Lin::ILinController* override;

    auto CreateDataPublisher(const std::string& canonicalName, const std::string& topic,
                             const std::string& mediaType,
                             const std::map<std::string, std::string>& labels, size_t history = 0)
        -> Services::PubSub::IDataPublisher* override;

    auto CreateDataSubscriber(const std::string& canonicalName, const std::string& topic,
                              const std::string& mediaType,
                              const std::map<std::string, std::string>& labels,
                              Services::PubSub::DataMessageHandlerT defaultDataHandler,
                              Services::PubSub::NewDataPublisherHandlerT newDataSourceHandler = nullptr)
        -> Services::PubSub::IDataSubscriber* override;

    auto CreateDataSubscriberInternal(const std::string& canonicalName, const std::string& linkName,
                                      const std::string& mediaType,
                                      const std::map<std::string, std::string>& publisherLabels,
                                      Services::PubSub::DataMessageHandlerT callback, Services::PubSub::IDataSubscriber* parent)
        -> Services::PubSub::DataSubscriberInternal* override;

    auto CreateRpcClient(const std::string& canonicalName, const std::string& functionName, const std::string& mediaType,
                         const std::map<std::string, std::string>& labels, Services::Rpc::RpcCallResultHandler handler)
        -> Services::Rpc::IRpcClient* override;

    auto CreateRpcServer(const std::string& canonicalName, const std::string& functionName, const std::string& mediaType,
                         const std::map<std::string, std::string>& labels, Services::Rpc::RpcCallHandler handler)
        -> Services::Rpc::IRpcServer* override;

    auto CreateRpcServerInternal(const std::string& functionName, const std::string& linkName,
                                 const std::string& mediaType, const std::map<std::string, std::string>& labels,
                                 Services::Rpc::RpcCallHandler handler, Services::Rpc::IRpcServer* parent)
        -> Services::Rpc::RpcServerInternal* override;

    void DiscoverRpcServers(const std::string& functionName, const std::string& mediaType,
                            const std::map<std::string, std::string>& labels,
                            Services::Rpc::RpcDiscoveryResultHandler handler) override;

    auto CreateSystemMonitor() -> Services::Orchestration::ISystemMonitor* override;
    auto GetSystemMonitor() -> Services::Orchestration::ISystemMonitor* override;
    auto CreateSystemController() -> Services::Orchestration::ISystemController* override;
    auto GetSystemController() -> Services::Orchestration::ISystemController* override;
    auto GetServiceDiscovery() -> Discovery::IServiceDiscovery* override;
    auto CreateLogger() -> Services::Logging::ILogger* override;
    auto GetLogger() -> Services::Logging::ILogger* override;
    auto CreateLifecycleServiceWithTimeSync() -> Services::Orchestration::ILifecycleServiceWithTimeSync* override;
    auto CreateLifecycleServiceNoTimeSync() -> Services::Orchestration::ILifecycleServiceNoTimeSync* override;
    auto GetLifecycleService() -> Services::Orchestration::ILifecycleServiceInternal* override;
    auto CreateTimeSyncService(Services::Orchestration::LifecycleService* service)
        -> Services::Orchestration::TimeSyncService* override;
    auto GetParticipantName() const -> const std::string& override { return _participantName; }

    void RegisterCanSimulator(Services::Can::IMsgForCanSimulator* busSim, const std::vector<std::string>& networkNames) override;
    void RegisterEthSimulator(Services::Ethernet::IMsgForEthSimulator* busSim, const std::vector<std::string>& networkNames) override;
    void RegisterFlexraySimulator(Services::Flexray::IMsgForFlexrayBusSimulator* busSim, const std::vector<std::string>& networkNames) override;
    void RegisterLinSimulator(Services::Lin::IMsgForLinSimulator* busSim, const std::vector<std::string>& networkNames) override;

    void SendMsg(const IServiceEndpoint* from, const Services::Can::WireCanFrameEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Can::CanFrameTransmitEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Can::CanControllerStatus& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Can::CanConfigureBaudrate& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Can::CanSetControllerMode& msg) override;

    void SendMsg(const IServiceEndpoint* from, const Services::Ethernet::WireEthernetFrameEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Ethernet::EthernetFrameTransmitEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Ethernet::EthernetStatus& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Ethernet::EthernetSetMode& msg) override;

    void SendMsg(const IServiceEndpoint* from, const Services::Flexray::WireFlexrayFrameEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Flexray::WireFlexrayFrameTransmitEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Flexray::FlexraySymbolEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Flexray::FlexraySymbolTransmitEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Flexray::FlexrayCycleStartEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Flexray::FlexrayHostCommand& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Flexray::FlexrayControllerConfig& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Flexray::FlexrayTxBufferConfigUpdate& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Flexray::WireFlexrayTxBufferUpdate& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Flexray::FlexrayPocStatusEvent& msg) override;

    void SendMsg(const IServiceEndpoint* from, const Services::Lin::LinSendFrameRequest& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Lin::LinSendFrameHeaderRequest& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Lin::LinTransmission& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Lin::LinWakeupPulse& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Lin::LinControllerConfig& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Lin::LinControllerStatusUpdate& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Lin::LinFrameResponseUpdate& msg) override;

    void SendMsg(const IServiceEndpoint*, const Services::Orchestration::NextSimTask& msg) override;
    void SendMsg(const IServiceEndpoint*, const Services::Orchestration::ParticipantStatus& msg) override;
    void SendMsg(const IServiceEndpoint*, const Services::Orchestration::ParticipantCommand& msg) override;
    void SendMsg(const IServiceEndpoint*, const Services::Orchestration::SystemCommand& msg) override;
    void SendMsg(const IServiceEndpoint*, const Services::Orchestration::WorkflowConfiguration& msg) override;

    void SendMsg(const IServiceEndpoint*, const Services::Logging::LogMsg& msg) override;
    void SendMsg(const IServiceEndpoint*, Services::Logging::LogMsg&& msg) override;

    void SendMsg(const IServiceEndpoint* from, const Services::PubSub::WireDataMessageEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Rpc::FunctionCall& msg) override;
    void SendMsg(const IServiceEndpoint* from, Services::Rpc::FunctionCall&& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Rpc::FunctionCallResponse& msg) override;
    void SendMsg(const IServiceEndpoint* from, Services::Rpc::FunctionCallResponse&& msg) override;

    void SendMsg(const IServiceEndpoint*, const Discovery::ParticipantDiscoveryEvent& msg) override;
    void SendMsg(const IServiceEndpoint*, const Discovery::ServiceDiscoveryEvent& msg) override;

    // targeted messaging
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Can::WireCanFrameEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Can::CanFrameTransmitEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Can::CanControllerStatus& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Can::CanConfigureBaudrate& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Can::CanSetControllerMode& msg) override;

    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Ethernet::WireEthernetFrameEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Ethernet::EthernetFrameTransmitEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Ethernet::EthernetStatus& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Ethernet::EthernetSetMode& msg) override;

    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::WireFlexrayFrameEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::WireFlexrayFrameTransmitEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::FlexraySymbolEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::FlexraySymbolTransmitEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::FlexrayCycleStartEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::FlexrayHostCommand& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::FlexrayControllerConfig& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::FlexrayTxBufferConfigUpdate& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::WireFlexrayTxBufferUpdate& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Flexray::FlexrayPocStatusEvent& msg) override;

    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Lin::LinSendFrameRequest& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Lin::LinSendFrameHeaderRequest& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Lin::LinTransmission& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Lin::LinWakeupPulse& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Lin::LinControllerConfig& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Lin::LinControllerStatusUpdate& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Lin::LinFrameResponseUpdate& msg) override;

    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName, const Services::Orchestration::NextSimTask& msg) override;
    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName, const Services::Orchestration::ParticipantStatus& msg) override;
    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName, const Services::Orchestration::ParticipantCommand& msg) override;
    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName, const Services::Orchestration::SystemCommand& msg) override;
    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName, const Services::Orchestration::WorkflowConfiguration& msg) override;

    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName, const Services::Logging::LogMsg& msg) override;
    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName, Services::Logging::LogMsg&& msg) override;

    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::PubSub::WireDataMessageEvent& msg) override;

    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Rpc::FunctionCall& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, Services::Rpc::FunctionCall&& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, const Services::Rpc::FunctionCallResponse& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, Services::Rpc::FunctionCallResponse&& msg) override;

    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName, const Discovery::ParticipantDiscoveryEvent& msg) override;
    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName, const Discovery::ServiceDiscoveryEvent& msg) override;

    void OnAllMessagesDelivered(std::function<void()> callback) override;
    void FlushSendBuffers() override;
    void ExecuteDeferred(std::function<void()> callback) override;

public:
    // ----------------------------------------
    // Public methods

    /*! \brief Join the middleware domain as a participant.
    *
    * Connect to the registry and join the domain
    * \param registryUri ID of the domain
    *
    * \throw std::exception A participant was created previously, or a
    * participant could not be created.
    */
    void JoinSilKitDomain(const std::string& registryUri) override;

    // For Testing Purposes:
    inline auto GetSilKitConnection() -> SilKitConnectionT& { return _connection; }

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
    void UpdateOptionalConfigValue(const std::string& canonicalName, SilKit::Util::Optional<ValueT>& configuredValue,
                                   const ValueT& passedValue);

    template <typename ValueT>
    void LogMismatchBetweenConfigAndPassedValue(const std::string& controllerName, const ValueT& passedValue,
                                                const ValueT& configuredValue);

    void OnSilKitDomainJoined();

    void SetupRemoteLogging();

    void SetTimeProvider(Services::Orchestration::ITimeProvider*);

    template<class SilKitMessageT>
    void SendMsgImpl(const IServiceEndpoint* from, SilKitMessageT&& msg);
    template<class SilKitMessageT>
    void SendMsgImpl(const IServiceEndpoint* from, const std::string& targetParticipantName, SilKitMessageT&& msg);

    template<class ControllerT>
    auto GetController(const std::string& networkName, const std::string& serviceName) -> ControllerT*;

    //!< internal services don't have a link config
    template<class ControllerT, typename... Arg>
    auto CreateInternalController(const std::string& serviceName, const Core::ServiceType serviceType,
                          const Core::SupplementalData& supplementalData, bool publishService, Arg&&... arg) -> ControllerT*;

    //!< Internal controller creation, explicit network argument for ConfigT without network
    template <class ConfigT, class ControllerT, typename... Arg>
    auto CreateController(const ConfigT& config, const std::string& network, const Core::ServiceType serviceType,
                          const Core::SupplementalData& supplementalData, bool publishService, Arg&&... arg)
        -> ControllerT*;

    //!< Internal controller creation, expects config.network
    template <class ConfigT, class ControllerT, typename... Arg>
    auto CreateController(const ConfigT& config, const Core::ServiceType serviceType,
                          const Core::SupplementalData& supplementalData, bool publishService, Arg&&... arg)
        -> ControllerT*;


    template<class IMsgForSimulatorT>
    void RegisterSimulator(IMsgForSimulatorT* busSim, Config::NetworkType linkType, const std::vector<std::string>& simulatedNetworkNames);

    template<class ConfigT>
    void AddTraceSinksToSource(ITraceMessageSource* controller, ConfigT config);

private:
    // ----------------------------------------
    // private members
    std::string _participantName;
    const SilKit::Config::ParticipantConfiguration _participantConfig;
    ParticipantId _participantId{0};

    Services::Orchestration::TimeProvider _timeProvider;

    std::unique_ptr<Services::Logging::ILogger> _logger;
    std::vector<std::unique_ptr<ITraceMessageSink>> _traceSinks;

    std::tuple<
        ControllerMap<Services::Can::IMsgForCanController>,
        ControllerMap<Services::Ethernet::IMsgForEthController>,
        ControllerMap<Services::Flexray::IMsgForFlexrayController>,
        ControllerMap<Services::Lin::IMsgForLinController>,
        ControllerMap<Services::PubSub::IMsgForDataPublisher>,
        ControllerMap<Services::PubSub::IMsgForDataSubscriber>,
        ControllerMap<Services::PubSub::IMsgForDataSubscriberInternal>,
        ControllerMap<Services::Rpc::IMsgForRpcClient>,
        ControllerMap<Services::Rpc::IMsgForRpcServer>,
        ControllerMap<Services::Rpc::IMsgForRpcServerInternal>,
        ControllerMap<Services::Logging::IMsgForLogMsgSender>,
        ControllerMap<Services::Logging::IMsgForLogMsgReceiver>,
        ControllerMap<Services::Orchestration::IMsgForLifecycleService>,
        ControllerMap<Services::Orchestration::IMsgForSystemMonitor>,
        ControllerMap<Services::Orchestration::IMsgForSystemController>,
        ControllerMap<Services::Orchestration::IMsgForTimeSyncService>,
        ControllerMap<Discovery::ServiceDiscovery>
    > _controllers;

    std::tuple<
        Services::Can::IMsgForCanSimulator*,
        Services::Ethernet::IMsgForEthSimulator*,
        Services::Flexray::IMsgForFlexrayBusSimulator*,
        Services::Lin::IMsgForLinSimulator*
    > _simulators {nullptr, nullptr, nullptr, nullptr};

    SilKitConnectionT _connection;

    // control variables to prevent multiple create accesses by public API 
    std::atomic<bool> _isSystemMonitorCreated{false};
    std::atomic<bool> _isSystemControllerCreated{false};
    std::atomic<bool> _isLoggerCreated{false};
    std::atomic<bool> _isLifecycleServiceCreated{false};
};

} // namespace Core
} // namespace SilKit

