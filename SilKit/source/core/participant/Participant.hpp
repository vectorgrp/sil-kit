/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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
#include "ReplayScheduler.hpp"
#include "Metrics.hpp"
#include "MetricsProcessor.hpp"
#include "IMetricsTimerThread.hpp"

// Interfaces relying on I_SilKit_Core_Internal
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

#include "IMsgForMetricsReceiver.hpp"
#include "IMsgForMetricsSender.hpp"

#include "ITraceMessageSink.hpp"
#include "ITraceMessageSource.hpp"

// core/service
#include "ServiceDiscovery.hpp"

// core/requests
#include "RequestReplyService.hpp"
#include "procs/ParticipantReplies.hpp"

#include "ProtocolVersion.hpp"
#include "TimeProvider.hpp"

// Add connection types here and make sure they are instantiated in Participant.cpp
#include "VAsioConnection.hpp"

// utilities for CreateController
#include "traits/SilKitServiceConfigTraits.hpp"

#include "NetworkSimulatorInternal.hpp"

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
    Participant(Config::ParticipantConfiguration participantConfig, ProtocolVersion version = CurrentProtocolVersion());

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
    auto CreateCanController(const std::string& canonicalName,
                             const std::string& networkName) -> Services::Can::ICanController* override;
    auto CreateEthernetController(const std::string& canonicalName,
                                  const std::string& networkName) -> Services::Ethernet::IEthernetController* override;
    auto CreateFlexrayController(const std::string& canonicalName,
                                 const std::string& networkName) -> Services::Flexray::IFlexrayController* override;
    auto CreateLinController(const std::string& canonicalName,
                             const std::string& networkName) -> Services::Lin::ILinController* override;

    auto CreateDataPublisher(const std::string& canonicalName, const SilKit::Services::PubSub::PubSubSpec& dataSpec,
                             size_t history = 0) -> Services::PubSub::IDataPublisher* override;

    auto CreateDataSubscriber(const std::string& canonicalName, const SilKit::Services::PubSub::PubSubSpec& dataSpec,
                              Services::PubSub::DataMessageHandler defaultDataHandler)
        -> Services::PubSub::IDataSubscriber* override;

    auto CreateDataSubscriberInternal(
        const std::string& canonicalName, const std::string& linkName, const std::string& mediaType,
        const std::vector<SilKit::Services::MatchingLabel>& publisherLabels,
        Services::PubSub::DataMessageHandler callback,
        Services::PubSub::IDataSubscriber* parent) -> Services::PubSub::DataSubscriberInternal* override;

    auto CreateRpcClient(const std::string& canonicalName, const SilKit::Services::Rpc::RpcSpec& dataSpec,
                         Services::Rpc::RpcCallResultHandler handler) -> Services::Rpc::IRpcClient* override;

    auto CreateRpcServer(const std::string& canonicalName, const SilKit::Services::Rpc::RpcSpec& dataSpec,
                         Services::Rpc::RpcCallHandler handler) -> Services::Rpc::IRpcServer* override;

    auto CreateRpcServerInternal(const std::string& functionName, const std::string& linkName,
                                 const std::string& mediaType,
                                 const std::vector<SilKit::Services::MatchingLabel>& clientLabels,
                                 Services::Rpc::RpcCallHandler handler,
                                 Services::Rpc::IRpcServer* parent) -> Services::Rpc::RpcServerInternal* override;

    auto CreateSystemMonitor() -> Services::Orchestration::ISystemMonitor* override;
    auto GetSystemMonitor() -> Services::Orchestration::ISystemMonitor* override;
    auto GetSystemController() -> Experimental::Services::Orchestration::ISystemController* override;
    auto GetServiceDiscovery() -> Discovery::IServiceDiscovery* override;
    auto GetRequestReplyService() -> RequestReply::IRequestReplyService* override;
    auto GetParticipantRepliesProcedure() -> RequestReply::IParticipantReplies* override;
    auto GetMetricsManager() -> IMetricsManager* override;

    auto GetLogger() -> Services::Logging::ILogger* override;
    auto CreateLifecycleService(Services::Orchestration::LifecycleConfiguration startConfiguration)
        -> Services::Orchestration::ILifecycleService* override;
    auto GetLifecycleService() -> Services::Orchestration::ILifecycleService* override;
    auto CreateTimeSyncService(Services::Orchestration::LifecycleService* lifecycleService)
        -> Services::Orchestration::TimeSyncService* override;

    auto CreateNetworkSimulator() -> Experimental::NetworkSimulation::INetworkSimulator* override;

    auto GetParticipantName() const -> const std::string& override
    {
        return _participantConfig.participantName;
    }
    auto GetRegistryUri() const -> const std::string& override
    {
        return _participantConfig.middleware.registryUri;
    }

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
    void SendMsg(const IServiceEndpoint* from, const Services::Lin::WireLinControllerConfig& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Lin::LinControllerStatusUpdate& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Lin::LinFrameResponseUpdate& msg) override;

    void SendMsg(const IServiceEndpoint*, const Services::Orchestration::NextSimTask& msg) override;
    void SendMsg(const IServiceEndpoint*, const Services::Orchestration::ParticipantStatus& msg) override;
    void SendMsg(const IServiceEndpoint*, const Services::Orchestration::SystemCommand& msg) override;
    void SendMsg(const IServiceEndpoint*, const Services::Orchestration::WorkflowConfiguration& msg) override;

    void SendMsg(const IServiceEndpoint*, const Services::Logging::LogMsg& msg) override;
    void SendMsg(const IServiceEndpoint*, Services::Logging::LogMsg&& msg) override;

    void SendMsg(const SilKit::Core::IServiceEndpoint* from, const VSilKit::MetricsUpdate& msg) override;

    void SendMsg(const IServiceEndpoint* from, const Services::PubSub::WireDataMessageEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Rpc::FunctionCall& msg) override;
    void SendMsg(const IServiceEndpoint* from, Services::Rpc::FunctionCall&& msg) override;
    void SendMsg(const IServiceEndpoint* from, const Services::Rpc::FunctionCallResponse& msg) override;
    void SendMsg(const IServiceEndpoint* from, Services::Rpc::FunctionCallResponse&& msg) override;

    void SendMsg(const IServiceEndpoint*, const Discovery::ParticipantDiscoveryEvent& msg) override;
    void SendMsg(const IServiceEndpoint*, const Discovery::ServiceDiscoveryEvent& msg) override;

    void SendMsg(const IServiceEndpoint*, const RequestReply::RequestReplyCall& msg) override;
    void SendMsg(const IServiceEndpoint*, const RequestReply::RequestReplyCallReturn& msg) override;

    // targeted messaging
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Can::WireCanFrameEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Can::CanFrameTransmitEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Can::CanControllerStatus& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Can::CanConfigureBaudrate& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Can::CanSetControllerMode& msg) override;

    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Ethernet::WireEthernetFrameEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Ethernet::EthernetFrameTransmitEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Ethernet::EthernetStatus& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Ethernet::EthernetSetMode& msg) override;

    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Flexray::WireFlexrayFrameEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Flexray::WireFlexrayFrameTransmitEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Flexray::FlexraySymbolEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Flexray::FlexraySymbolTransmitEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Flexray::FlexrayCycleStartEvent& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Flexray::FlexrayHostCommand& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Flexray::FlexrayControllerConfig& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Flexray::FlexrayTxBufferConfigUpdate& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Flexray::WireFlexrayTxBufferUpdate& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Flexray::FlexrayPocStatusEvent& msg) override;

    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Lin::LinSendFrameRequest& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Lin::LinSendFrameHeaderRequest& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Lin::LinTransmission& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Lin::LinWakeupPulse& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Lin::WireLinControllerConfig& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Lin::LinControllerStatusUpdate& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Lin::LinFrameResponseUpdate& msg) override;

    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName,
                 const Services::Orchestration::NextSimTask& msg) override;
    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName,
                 const Services::Orchestration::ParticipantStatus& msg) override;
    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName,
                 const Services::Orchestration::SystemCommand& msg) override;
    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName,
                 const Services::Orchestration::WorkflowConfiguration& msg) override;

    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName,
                 const Services::Logging::LogMsg& msg) override;
    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName,
                 Services::Logging::LogMsg&& msg) override;

    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName,
                 const VSilKit::MetricsUpdate& msg) override;

    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::PubSub::WireDataMessageEvent& msg) override;

    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Rpc::FunctionCall& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 Services::Rpc::FunctionCall&& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 const Services::Rpc::FunctionCallResponse& msg) override;
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName,
                 Services::Rpc::FunctionCallResponse&& msg) override;

    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName,
                 const Discovery::ParticipantDiscoveryEvent& msg) override;
    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName,
                 const Discovery::ServiceDiscoveryEvent& msg) override;

    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName,
                 const RequestReply::RequestReplyCall& msg) override;
    void SendMsg(const IServiceEndpoint*, const std::string& targetParticipantName,
                 const RequestReply::RequestReplyCallReturn& msg) override;

    void OnAllMessagesDelivered(std::function<void()> callback) override;
    void FlushSendBuffers() override;
    void ExecuteDeferred(std::function<void()> callback) override;

    void AddAsyncSubscriptionsCompletionHandler(std::function<void()> handler) override;

    void SetIsSystemControllerCreated(bool isCreated) override;
    bool GetIsSystemControllerCreated() override;

    void SetIsNetworkSimulatorCreated(bool isCreated) override;
    bool GetIsNetworkSimulatorCreated() override;

    size_t GetNumberOfConnectedParticipants() override;

    size_t GetNumberOfRemoteReceivers(const IServiceEndpoint* service, const std::string& msgTypeName) override;
    std::vector<std::string> GetParticipantNamesOfRemoteReceivers(const IServiceEndpoint* service,
                                                                  const std::string& msgTypeName) override;

    void NotifyShutdown() override;
    void EvaluateAggregationInfo(bool isSyncSimStepHandler) override;

    void RegisterReplayController(SilKit::Tracing::IReplayDataController* replayController,
                                  const std::string& controllerName,
                                  const SilKit::Config::SimulatedNetwork& simulatedNetwork) override;
    bool ParticipantHasCapability(const std::string& /*participantName*/,
                                  const std::string& /*capability*/) const override;

    std::string GetServiceDescriptorString(
        SilKit::Experimental::NetworkSimulation::ControllerDescriptor controllerDescriptor) override;
    auto GetLoggerInternal() -> Services::Logging::ILoggerInternal* override;


    auto GetMetricsProcessor() -> IMetricsProcessor* override;
    auto GetMetricsSender() -> IMetricsSender* override;

public:
    // ----------------------------------------
    // Public methods

    /*! \brief Connect to the registry and join the simulation.
    *
    * \throw SilKit::SilKitError A participant was created previously, or a
    * participant could not be created.
    */
    void JoinSilKitSimulation() override;

    // For Testing Purposes:
    inline auto GetSilKitConnection() -> SilKitConnectionT&
    {
        return _connection;
    }

private:
    // ----------------------------------------
    // private datatypes
    template <typename ControllerT>
    using ControllerMap = std::unordered_map<std::string, std::unique_ptr<ControllerT>>;

private:
    // ----------------------------------------
    // private methods

    //!< Search for the controller configuration by name and set configured values. Initialize with controller name if no config is found.
    template <typename ConfigT>
    auto GetConfigByControllerName(const std::vector<ConfigT>& controllers,
                                   const std::string& canonicalName) -> ConfigT;

    //!< Update the controller configuration for a given optional field. Prefers configured values over programmatically passed values.
    template <typename ValueT>
    void UpdateOptionalConfigValue(const std::string& canonicalName, SilKit::Util::Optional<ValueT>& configuredValue,
                                   const ValueT& passedValue);

    template <typename ValueT>
    void LogMismatchBetweenConfigAndPassedValue(const std::string& controllerName, const ValueT& passedValue,
                                                const ValueT& configuredValue);

    void OnSilKitSimulationJoined();

    void SetupRemoteLogging();
    void SetupMetrics();

    void SetTimeProvider(Services::Orchestration::ITimeProvider*);

    template <class SilKitMessageT>
    void SendMsgImpl(const IServiceEndpoint* from, SilKitMessageT&& msg);
    template <class SilKitMessageT>
    void SendMsgImpl(const IServiceEndpoint* from, const std::string& targetParticipantName, SilKitMessageT&& msg);

    template <class ControllerT>
    auto GetController(const std::string& serviceName) -> ControllerT*;

    //!< Internal controller creation, explicit network argument for ConfigT without network
    template <class ControllerT, typename... Arg>
    auto CreateController(const SilKitServiceTraitConfigType_t<ControllerT>& config, const std::string& network,
                          const Core::SupplementalData& supplementalData, bool publishServiceDiscovery,
                          bool registerSilKitService, Arg&&... arg) -> ControllerT*;

    //!< Internal controller creation, expects config.network
    template <class ControllerT, typename... Arg>
    auto CreateController(const SilKitServiceTraitConfigType_t<ControllerT>& config,
                          const Core::SupplementalData& supplementalData, bool publishServiceDiscovery,
                          bool registerSilKitService, Arg&&... arg) -> ControllerT*;

    //!< Internal late controller registration. Used for TimeSyncService to create the controller
    //! and only register message reception later if really needed.
    void RegisterTimeSyncService(SilKit::Services::Orchestration::TimeSyncService* controllerPtr) override;

    void RegisterSimulator(ISimulator* busSim, std::string networkName,
                           Experimental::NetworkSimulation::SimulatedNetworkType networkType) override;

    void AddTraceSinksToSource(ITraceMessageSource* controller, SilKit::Config::SimulatedNetwork config) override;

    template <class ConfigT>
    void AddTraceSinksToSourceInternal(ITraceMessageSource* controller, ConfigT config);

    auto GetOrCreateMetricsSender() -> VSilKit::IMetricsSender*;

    void CreateSystemInformationMetrics();

    auto MakeTimerThread() -> std::unique_ptr<IMetricsTimerThread>;

private:
    // ----------------------------------------
    // private members
    const SilKit::Config::ParticipantConfiguration _participantConfig;
    ParticipantId _participantId{0};

    Services::Orchestration::TimeProvider _timeProvider;

    std::unique_ptr<Services::Logging::ILoggerInternal> _logger;
    std::vector<std::unique_ptr<ITraceMessageSink>> _traceSinks;
    std::unique_ptr<Tracing::ReplayScheduler> _replayScheduler;
    std::unique_ptr<RequestReply::ParticipantReplies> _participantReplies;

    std::tuple<
        ControllerMap<Services::Can::IMsgForCanController>, ControllerMap<Services::Ethernet::IMsgForEthController>,
        ControllerMap<Services::Flexray::IMsgForFlexrayController>, ControllerMap<Services::Lin::IMsgForLinController>,
        ControllerMap<Services::PubSub::IMsgForDataPublisher>, ControllerMap<Services::PubSub::IMsgForDataSubscriber>,
        ControllerMap<Services::PubSub::IMsgForDataSubscriberInternal>, ControllerMap<Services::Rpc::IMsgForRpcClient>,
        ControllerMap<Services::Rpc::IMsgForRpcServer>, ControllerMap<Services::Rpc::IMsgForRpcServerInternal>,
        ControllerMap<Services::Logging::IMsgForLogMsgSender>, ControllerMap<Services::Logging::IMsgForLogMsgReceiver>,
        ControllerMap<Services::Orchestration::IMsgForLifecycleService>,
        ControllerMap<Services::Orchestration::IMsgForSystemMonitor>,
        ControllerMap<Services::Orchestration::IMsgForSystemController>,
        ControllerMap<Services::Orchestration::IMsgForTimeSyncService>, ControllerMap<Discovery::ServiceDiscovery>,
        ControllerMap<RequestReply::RequestReplyService>, ControllerMap<IMsgForMetricsReceiver>,
        ControllerMap<IMsgForMetricsSender>>
        _controllers;

    std::atomic<EndpointId> _localEndpointId{0};

    std::unique_ptr<Experimental::NetworkSimulation::NetworkSimulatorInternal> _networkSimulatorInternal;

    std::unique_ptr<IMetricsProcessor> _metricsProcessor;
    std::unique_ptr<IMetricsManager> _metricsManager;

    IMetricsSender* _metricsSender{nullptr};

    std::tuple<Services::Can::IMsgForCanSimulator*, Services::Ethernet::IMsgForEthSimulator*,
               Services::Flexray::IMsgForFlexrayBusSimulator*, Services::Lin::IMsgForLinSimulator*>
        _simulators{nullptr, nullptr, nullptr, nullptr};

    SilKitConnectionT _connection;

    // NB: Must be destroyed before _connection and _metricsManager
    std::unique_ptr<IMetricsTimerThread> _metricsTimerThread;

    // control variables to prevent multiple create accesses by public API
    std::atomic<bool> _isSystemMonitorCreated{false};
    std::atomic<bool> _isSystemControllerCreated{false};
    std::atomic<bool> _isLoggerCreated{false};
    std::atomic<bool> _isLifecycleServiceCreated{false};
    std::atomic<bool> _isNetworkSimulatorCreated{false};
};

} // namespace Core
} // namespace SilKit
