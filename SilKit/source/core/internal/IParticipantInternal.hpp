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

#include <atomic>

#include "silkit/participant/IParticipant.hpp"
#include "silkit/experimental/services/orchestration/ISystemController.hpp"
#include "silkit/experimental/netsim/NetworkSimulatorDatatypes.hpp"


#include "internal_fwd.hpp"
#include "IServiceEndpoint.hpp"
#include "ServiceDatatypes.hpp"
#include "RequestReplyDatatypes.hpp"
#include "OrchestrationDatatypes.hpp"
#include "LoggingDatatypesInternal.hpp"
#include "MetricsDatatypes.hpp"

#include "WireCanMessages.hpp"
#include "WireDataMessages.hpp"
#include "WireEthernetMessages.hpp"
#include "WireFlexrayMessages.hpp"
#include "WireLinMessages.hpp"
#include "WireRpcMessages.hpp"
#include "Metrics.hpp"

#include "ISimulator.hpp"
#include "IReplayDataController.hpp"
#include "ITraceMessageSource.hpp"

// forwards
namespace SilKit {
namespace Services {
namespace Logging {
struct ILoggerInternal;
}
} // namespace Services
namespace Tracing {
class ReplayScheduler;
} // namespace Tracing
} // namespace SilKit

namespace SilKit {
namespace Core {

class IParticipantInternal : public IParticipant
{
public:
    // ----------------------------------------
    // Public methods
    virtual auto GetParticipantName() const -> const std::string& = 0;

    /*! \brief Returns the URI of the registry this participant is connecting to.
     *
     * The URI must be specified in the configuration (which has priority) or the CreateParticipant call.
     *
     * @return the URI of the registry
     */
    virtual auto GetRegistryUri() const -> const std::string& = 0;

    /*! \brief Connect to the registry and join the simulation.
    *
    * \throw SilKit::SilKitError A participant was created previously, or a
    * participant could not be created.
    */
    virtual void JoinSilKitSimulation() = 0;

    // Register subscriptions for the time sync service only when the user actually calls CreateTimeSyncService.
    virtual void RegisterTimeSyncService(SilKit::Services::Orchestration::TimeSyncService* controllerPtr) = 0;

    // For NetworkSimulator integration:
    virtual void RegisterSimulator(ISimulator* busSim, std::string networkName,
                                   SilKit::Experimental::NetworkSimulation::SimulatedNetworkType networkType) = 0;

    virtual void AddTraceSinksToSource(ITraceMessageSource* traceSource, SilKit::Config::SimulatedNetwork config) = 0;

    // The SendMsgs are virtual functions so we can mock them in testing.
    // For performance reasons this may change in the future.

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Can::WireCanFrameEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Can::CanFrameTransmitEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Can::CanControllerStatus& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Can::CanConfigureBaudrate& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Can::CanSetControllerMode& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Ethernet::WireEthernetFrameEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Ethernet::EthernetFrameTransmitEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Ethernet::EthernetStatus& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Ethernet::EthernetSetMode& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Flexray::WireFlexrayFrameEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Flexray::WireFlexrayFrameTransmitEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Flexray::FlexraySymbolEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Flexray::FlexraySymbolTransmitEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Flexray::FlexrayCycleStartEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Flexray::FlexrayHostCommand& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Flexray::FlexrayControllerConfig& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Flexray::FlexrayTxBufferConfigUpdate& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Flexray::WireFlexrayTxBufferUpdate& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Flexray::FlexrayPocStatusEvent& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Lin::LinSendFrameRequest& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Lin::LinSendFrameHeaderRequest& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Lin::LinTransmission& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Lin::LinWakeupPulse& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Lin::WireLinControllerConfig& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Lin::LinControllerStatusUpdate& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Lin::LinFrameResponseUpdate& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::PubSub::WireDataMessageEvent& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Rpc::FunctionCall& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, Services::Rpc::FunctionCall&& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Rpc::FunctionCallResponse& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, Services::Rpc::FunctionCallResponse&& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Orchestration::NextSimTask& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Orchestration::ParticipantStatus& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Orchestration::SystemCommand& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Services::Orchestration::WorkflowConfiguration& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Services::Logging::LogMsg& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, Services::Logging::LogMsg&& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const VSilKit::MetricsUpdate& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const Discovery::ParticipantDiscoveryEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const Discovery::ServiceDiscoveryEvent& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const RequestReply::RequestReplyCall& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from,
                         const RequestReply::RequestReplyCallReturn& msg) = 0;

    // targeted messaging
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Can::WireCanFrameEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Can::CanFrameTransmitEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Can::CanControllerStatus& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Can::CanConfigureBaudrate& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Can::CanSetControllerMode& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Ethernet::WireEthernetFrameEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Ethernet::EthernetFrameTransmitEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Ethernet::EthernetStatus& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Ethernet::EthernetSetMode& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Flexray::WireFlexrayFrameEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Flexray::WireFlexrayFrameTransmitEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Flexray::FlexraySymbolEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Flexray::FlexraySymbolTransmitEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Flexray::FlexrayCycleStartEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Flexray::FlexrayHostCommand& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Flexray::FlexrayControllerConfig& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Flexray::FlexrayTxBufferConfigUpdate& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Flexray::WireFlexrayTxBufferUpdate& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Flexray::FlexrayPocStatusEvent& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Lin::LinSendFrameRequest& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Lin::LinSendFrameHeaderRequest& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Lin::LinTransmission& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Lin::LinWakeupPulse& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Lin::WireLinControllerConfig& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Lin::LinControllerStatusUpdate& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Lin::LinFrameResponseUpdate& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::PubSub::WireDataMessageEvent& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Rpc::FunctionCall& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         Services::Rpc::FunctionCall&& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Rpc::FunctionCallResponse& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         Services::Rpc::FunctionCallResponse&& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Orchestration::NextSimTask& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Orchestration::ParticipantStatus& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Orchestration::SystemCommand& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Orchestration::WorkflowConfiguration& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Services::Logging::LogMsg& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         Services::Logging::LogMsg&& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const VSilKit::MetricsUpdate& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Discovery::ParticipantDiscoveryEvent& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const Discovery::ServiceDiscoveryEvent& msg) = 0;

    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const RequestReply::RequestReplyCall& msg) = 0;
    virtual void SendMsg(const SilKit::Core::IServiceEndpoint* from, const std::string& targetParticipantName,
                         const RequestReply::RequestReplyCallReturn& msg) = 0;

    // For Connection/middleware support:
    virtual void OnAllMessagesDelivered(std::function<void()> callback) = 0;
    virtual void FlushSendBuffers() = 0;
    virtual void ExecuteDeferred(std::function<void()> callback) = 0;

    // Service discovery for dynamic, configuration-less simulations
    virtual auto GetServiceDiscovery() -> Discovery::IServiceDiscovery* = 0;
    // Request replay service for internal RPC and barriers
    virtual auto GetRequestReplyService() -> RequestReply::IRequestReplyService* = 0;
    virtual auto GetParticipantRepliesProcedure() -> RequestReply::IParticipantReplies* = 0;

    // Internal DataSubscriber that is only created on a matching data connection
    virtual auto CreateDataSubscriberInternal(
        const std::string& topic, const std::string& linkName, const std::string& mediaType,
        const std::vector<SilKit::Services::MatchingLabel>& publisherLabels,
        Services::PubSub::DataMessageHandler callback,
        Services::PubSub::IDataSubscriber* parent) -> Services::PubSub::DataSubscriberInternal* = 0;

    // Internal Rpc server that is only created on a matching rpc connection
    virtual auto CreateRpcServerInternal(
        const std::string& functionName, const std::string& linkName, const std::string& mediaType,
        const std::vector<SilKit::Services::MatchingLabel>& labels, Services::Rpc::RpcCallHandler handler,
        Services::Rpc::IRpcServer* parent) -> SilKit::Services::Rpc::RpcServerInternal* = 0;

    //! \brief Return the ISystemMonitor at this SIL Kit participant.
    virtual auto GetSystemMonitor() -> Services::Orchestration::ISystemMonitor* = 0;

    //! \brief Return the ISystemController at this SIL Kit participant.
    virtual auto GetSystemController() -> Experimental::Services::Orchestration::ISystemController* = 0;

    //! \brief Return the ILogger at this SIL Kit participant.
    virtual auto GetLogger() -> Services::Logging::ILogger* = 0;

    //! \brief Return the LifecycleService at this SIL Kit participant.
    virtual auto GetLifecycleService() -> Services::Orchestration::ILifecycleService* = 0;

    //! \brief Create the ITimeSyncService for the given lifecycle service (one time per lifecycle service).
    virtual auto CreateTimeSyncService(Services::Orchestration::LifecycleService* lifecycleService)
        -> Services::Orchestration::TimeSyncService* = 0;

    virtual auto CreateNetworkSimulator() -> Experimental::NetworkSimulation::INetworkSimulator* = 0;

    virtual auto GetMetricsManager() -> IMetricsManager* = 0;

    // Register handlers for completion of async service creation
    virtual void AddAsyncSubscriptionsCompletionHandler(std::function<void()> handler) = 0;

    virtual bool GetIsSystemControllerCreated() = 0;
    virtual void SetIsSystemControllerCreated(bool isCreated) = 0;

    virtual bool GetIsNetworkSimulatorCreated() = 0;
    virtual void SetIsNetworkSimulatorCreated(bool isCreated) = 0;

    virtual size_t GetNumberOfConnectedParticipants() = 0;
    virtual size_t GetNumberOfRemoteReceivers(const IServiceEndpoint* service, const std::string& msgTypeName) = 0;
    virtual std::vector<std::string> GetParticipantNamesOfRemoteReceivers(const IServiceEndpoint* service,
                                                                          const std::string& msgTypeName) = 0;

    virtual void NotifyShutdown() = 0;
    virtual void EvaluateAggregationInfo(bool isSyncSimStepHandler) = 0;

    virtual void RegisterReplayController(SilKit::Tracing::IReplayDataController* replayController,
                                          const std::string& controllerName,
                                          const SilKit::Config::SimulatedNetwork& simulatedNetwork) = 0;
    virtual bool ParticipantHasCapability(const std::string& participantName, const std::string& capability) const = 0;

    virtual std::string GetServiceDescriptorString(
        SilKit::Experimental::NetworkSimulation::ControllerDescriptor controllerDescriptor) = 0;

    virtual auto GetLoggerInternal() -> Services::Logging::ILoggerInternal* = 0;

    virtual auto GetMetricsProcessor() -> IMetricsProcessor* = 0;
    virtual auto GetMetricsSender() -> IMetricsSender* = 0;
};

} // namespace Core
} // namespace SilKit
