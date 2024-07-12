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

#include <chrono>
#include <string>
#include <unordered_map>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"
#include "silkit/services/logging/LoggingDatatypes.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/orchestration/ILifecycleService.hpp"
#include "silkit/services/orchestration/ITimeSyncService.hpp"
#include "silkit/services/orchestration/ISystemMonitor.hpp"
#include "silkit/experimental/services/orchestration/ISystemController.hpp"

#include "silkit/services/fwd_decl.hpp"
#include "silkit/services/can/CanDatatypes.hpp"
#include "silkit/services/ethernet/EthernetDatatypes.hpp"
#include "silkit/services/flexray/FlexrayDatatypes.hpp"
#include "silkit/services/lin/LinDatatypes.hpp"
#include "silkit/services/pubsub/PubSubDatatypes.hpp"
#include "silkit/services/rpc/RpcDatatypes.hpp"

#include "silkit/experimental/netsim/INetworkSimulator.hpp"

#include "IParticipantInternal.hpp"
#include "IServiceDiscovery.hpp"
#include "IRequestReplyService.hpp"
#include "IRequestReplyProcedure.hpp"
#include "procs/IParticipantReplies.hpp"
#include "LifecycleService.hpp"
#include "SynchronizedHandlers.hpp"
#include "MockTimeProvider.hpp"

#include "MockLogger.hpp"

namespace SilKit {
namespace Core {
namespace Tests {

using SilKit::Util::HandlerId;

using SilKit::Services::Logging::MockLogger;

class MockLifecycleService : public Services::Orchestration::ILifecycleService
{
public:
    MOCK_METHOD(void, SetCommunicationReadyHandler, (SilKit::Services::Orchestration::CommunicationReadyHandler),
                (override));
    MOCK_METHOD(void, SetCommunicationReadyHandlerAsync, (SilKit::Services::Orchestration::CommunicationReadyHandler),
                (override));
    MOCK_METHOD(void, CompleteCommunicationReadyHandlerAsync, (), (override));
    MOCK_METHOD(void, SetStartingHandler, (SilKit::Services::Orchestration::StartingHandler), (override));
    MOCK_METHOD(void, SetStopHandler, (SilKit::Services::Orchestration::StopHandler), (override));
    MOCK_METHOD(void, SetShutdownHandler, (SilKit::Services::Orchestration::ShutdownHandler), (override));
    MOCK_METHOD(void, SetAbortHandler, (SilKit::Services::Orchestration::AbortHandler), (override));
    MOCK_METHOD(std::future<Services::Orchestration::ParticipantState>, StartLifecycle, (), (override));
    MOCK_METHOD(void, ReportError, (std::string /*errorMsg*/), (override));
    MOCK_METHOD(void, Pause, (std::string /*reason*/), (override));
    MOCK_METHOD(void, Continue, (), (override));
    MOCK_METHOD(void, Stop, (std::string /*reason*/), (override));
    MOCK_METHOD(Services::Orchestration::ParticipantState, State, (), (override, const));
    MOCK_METHOD(Services::Orchestration::ParticipantStatus&, Status, (), (override, const));
    MOCK_METHOD(Services::Orchestration::ITimeSyncService*, GetTimeSyncService, (), ());
    MOCK_METHOD(Services::Orchestration::ITimeSyncService*, CreateTimeSyncService, (), (override));
    MOCK_METHOD(void, AddAsyncSubscriptionsCompletionHandler, (std::function<void()> /*handler*/));
    MOCK_METHOD(Services::Orchestration::OperationMode, GetOperationMode, (), (const));
};

class MockTimeSyncService : public Services::Orchestration::ITimeSyncService
{
public:
    MOCK_METHOD(void, SetSimulationStepHandler, (SimulationStepHandler task, std::chrono::nanoseconds initialStepSize),
                (override));
    MOCK_METHOD(void, SetSimulationStepHandlerAsync,
                (SimulationStepHandler task, std::chrono::nanoseconds initialStepSize), (override));
    MOCK_METHOD(void, CompleteSimulationStep, (), (override));
    MOCK_METHOD(std::chrono::nanoseconds, Now, (), (override, const));
};

class MockSystemMonitor : public Services::Orchestration::ISystemMonitor
{
public:
    MOCK_METHOD(HandlerId, AddSystemStateHandler, (SystemStateHandler));
    MOCK_METHOD(void, RemoveSystemStateHandler, (HandlerId));

    MOCK_METHOD(HandlerId, AddParticipantStatusHandler, (ParticipantStatusHandler));
    MOCK_METHOD(void, RemoveParticipantStatusHandler, (HandlerId));

    MOCK_CONST_METHOD0(SystemState, Services::Orchestration::SystemState());
    MOCK_CONST_METHOD1(ParticipantStatus,
                       const Services::Orchestration::ParticipantStatus&(const std::string& participantName));

    MOCK_METHOD(void, SetParticipantConnectedHandler, (ParticipantConnectedHandler handler), (override));
    MOCK_METHOD(void, SetParticipantDisconnectedHandler, (ParticipantDisconnectedHandler handler), (override));
    MOCK_METHOD(bool, IsParticipantConnected, (const std::string& participantName), (const, override));
};

class MockSystemController : public Experimental::Services::Orchestration::ISystemController
{
public:
    MOCK_CONST_METHOD0(Run, void());
    MOCK_CONST_METHOD0(Stop, void());
    MOCK_CONST_METHOD0(AbortSimulation, void());
    MOCK_METHOD((void), SetWorkflowConfiguration,
                (const SilKit::Services::Orchestration::WorkflowConfiguration& workflowConfiguration));
};

class MockServiceDiscovery : public Discovery::IServiceDiscovery
{
public:
    MOCK_METHOD(void, NotifyServiceCreated, (const ServiceDescriptor& serviceDescriptor), (override));
    MOCK_METHOD(void, NotifyServiceRemoved, (const ServiceDescriptor& serviceDescriptor), (override));
    MOCK_METHOD(void, RegisterServiceDiscoveryHandler, (SilKit::Core::Discovery::ServiceDiscoveryHandler handler),
                (override));
    MOCK_METHOD(void, RegisterSpecificServiceDiscoveryHandler,
                (SilKit::Core::Discovery::ServiceDiscoveryHandler handler, const std::string& controllerType,
                 const std::string& topic, const std::vector<SilKit::Services::MatchingLabel>& labels),
                (override));
    MOCK_METHOD(std::vector<ServiceDescriptor>, GetServices, (), (const, override));
    MOCK_METHOD(void, OnParticpantRemoval, (const std::string& participantName), (override));
};

class MockRequestReplyService : public RequestReply::IRequestReplyService
{
public:
    MOCK_METHOD((Util::Uuid), Call, (RequestReply::FunctionType functionType, std::vector<uint8_t> callData),
                (override));
    MOCK_METHOD(void, SubmitCallReturn,
                (Util::Uuid callUuid, RequestReply::FunctionType functionType, std::vector<uint8_t> callReturnData,
                 RequestReply::CallReturnStatus callReturnStatus),
                (override));
};


class MockParticipantReplies : public RequestReply::IParticipantReplies
{
public:
    void CallAfterAllParticipantsReplied(std::function<void()> completionFunction) override
    {
        // Directly trigger
        completionFunction();
    }
};

class DummyNetworkSimulator : public Experimental::NetworkSimulation::INetworkSimulator
{
public:
    void SimulateNetwork(
        const std::string& /*networkName*/, Experimental::NetworkSimulation::SimulatedNetworkType /*networkType*/,
        std::unique_ptr<Experimental::NetworkSimulation::ISimulatedNetwork> /*simulatedNetwork*/) override
    {
    }

    void Start() override {}
};

class DummyMetricsManager : public IMetricsManager
{
    class DummyCounterMetric : public ICounterMetric
    {
    public:
        void Add(uint64_t /* delta */) override {}
        void Set(uint64_t /* value */) override {}
    };

    class DummyStatisticMetric : public IStatisticMetric
    {
    public:
        void Take(double /* value */) override {}
    };

    class DummyStringListMetric : public IStringListMetric
    {
    public:
        void Clear() override {}
        void Add(const std::string&) override {}
    };

public:
    auto GetCounter(const std::string& name) -> ICounterMetric* override
    {
        auto it = _counters.find(name);
        if (it == _counters.end())
        {
            it = _counters.emplace().first;
        }
        return &(it->second);
    }

    auto GetStatistic(const std::string& name) -> IStatisticMetric* override
    {
        auto it = _statistics.find(name);
        if (it == _statistics.end())
        {
            it = _statistics.emplace().first;
        }
        return &(it->second);
    }

    auto GetStringList(const std::string& name) -> IStringListMetric* override
    {
        auto it = _stringLists.find(name);
        if (it == _stringLists.end())
        {
            it = _stringLists.emplace().first;
        }
        return &(it->second);
    }

    void SubmitUpdates() override {}

private:
    std::unordered_map<std::string, DummyCounterMetric> _counters;
    std::unordered_map<std::string, DummyStatisticMetric> _statistics;
    std::unordered_map<std::string, DummyStringListMetric> _stringLists;
};

class DummyParticipant : public IParticipantInternal
{
public:
    DummyParticipant()
    {
        ON_CALL(mockLifecycleService, GetTimeSyncService).WillByDefault(testing::Return(&mockTimeSyncService));
        ON_CALL(mockLifecycleService, CreateTimeSyncService).WillByDefault(testing::Return(&mockTimeSyncService));
        ON_CALL(logger, GetLogLevel()).WillByDefault(testing::Return(Services::Logging::Level::Debug));
    }

    auto CreateCanController(const std::string& /*canonicalName*/,
                             const std::string& /*networkName*/) -> Services::Can::ICanController* override
    {
        return nullptr;
    }
    auto CreateEthernetController(const std::string& /*canonicalName*/, const std::string& /*networkName*/)
        -> Services::Ethernet::IEthernetController* override
    {
        return nullptr;
    }
    auto CreateFlexrayController(const std::string& /*canonicalName*/,
                                 const std::string& /*networkName*/) -> Services::Flexray::IFlexrayController* override
    {
        return nullptr;
    }
    auto CreateLinController(const std::string& /*canonicalName*/,
                             const std::string& /*networkName*/) -> Services::Lin::ILinController* override
    {
        return nullptr;
    }
    auto CreateDataPublisher(const std::string& /*canonicalName*/,
                             const SilKit::Services::PubSub::PubSubSpec& /*dataSpec*/,
                             size_t /*history*/ = 0) -> SilKit::Services::PubSub::IDataPublisher* override
    {
        return nullptr;
    }
    auto CreateDataSubscriber(const std::string& /*canonicalName*/,
                              const SilKit::Services::PubSub::PubSubSpec& /*dataSpec*/,
                              Services::PubSub::DataMessageHandler /*dataMessageHandler*/)
        -> SilKit::Services::PubSub::IDataSubscriber* override
    {
        return nullptr;
    }
    auto CreateDataSubscriberInternal(
        const std::string& /*topic*/, const std::string& /*linkName*/, const std::string& /*mediaType*/,
        const std::vector<SilKit::Services::MatchingLabel>& /*publisherLabels*/,
        Services::PubSub::DataMessageHandler /*callback*/,
        Services::PubSub::IDataSubscriber* /*parent*/) -> Services::PubSub::DataSubscriberInternal* override
    {
        return nullptr;
    }

    auto CreateRpcClient(const std::string& /*controllerName*/, const SilKit::Services::Rpc::RpcSpec& /*dataSpec*/,
                         SilKit::Services::Rpc::RpcCallResultHandler /*handler*/)
        -> SilKit::Services::Rpc::IRpcClient* override
    {
        return nullptr;
    }
    auto CreateRpcServer(const std::string& /*controllerName*/, const SilKit::Services::Rpc::RpcSpec& /*dataSpec*/,
                         SilKit::Services::Rpc::RpcCallHandler /*handler*/)
        -> SilKit::Services::Rpc::IRpcServer* override
    {
        return nullptr;
    }
    auto CreateRpcServerInternal(
        const std::string& /*functionName*/, const std::string& /*linkName*/, const std::string& /*mediaType*/,
        const std::vector<SilKit::Services::MatchingLabel>& /*labels*/, Services::Rpc::RpcCallHandler /*handler*/,
        Services::Rpc::IRpcServer* /*parent*/) -> SilKit::Services::Rpc::RpcServerInternal* override
    {
        return nullptr;
    }

    auto GetLifecycleService() -> Services::Orchestration::ILifecycleService* override
    {
        return &mockLifecycleService;
    }
    auto CreateLifecycleService(Services::Orchestration::LifecycleConfiguration)
        -> Services::Orchestration::ILifecycleService* override
    {
        return &mockLifecycleService;
    }

    auto CreateNetworkSimulator() -> Experimental::NetworkSimulation::INetworkSimulator* override
    {
        return &mockNetworkSimulator;
    }

    MOCK_METHOD(Services::Orchestration::TimeSyncService*, CreateTimeSyncService,
                (Services::Orchestration::LifecycleService*), (override));
    auto GetSystemMonitor() -> Services::Orchestration::ISystemMonitor* override
    {
        return &mockSystemMonitor;
    }
    auto CreateSystemMonitor() -> Services::Orchestration::ISystemMonitor* override
    {
        return &mockSystemMonitor;
    }
    auto GetSystemController() -> Experimental::Services::Orchestration::ISystemController* override
    {
        return &mockSystemController;
    }

    auto GetLogger() -> Services::Logging::ILogger* override
    {
        return &logger;
    }

    void RegisterSimulator(Core::ISimulator*, std::string,
                           Experimental::NetworkSimulation::SimulatedNetworkType) override
    {
    }

    void AddTraceSinksToSource(ITraceMessageSource*, SilKit::Config::SimulatedNetwork) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Can::WireCanFrameEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Can::CanFrameTransmitEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Can::CanControllerStatus& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Can::CanConfigureBaudrate& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Can::CanSetControllerMode& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Ethernet::WireEthernetFrameEvent& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/,
                 const Services::Ethernet::EthernetFrameTransmitEvent& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Ethernet::EthernetStatus& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Ethernet::EthernetSetMode& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Flexray::WireFlexrayFrameEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/,
                 const Services::Flexray::WireFlexrayFrameTransmitEvent& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Flexray::FlexraySymbolEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/,
                 const Services::Flexray::FlexraySymbolTransmitEvent& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Flexray::FlexrayCycleStartEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Flexray::FlexrayHostCommand& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Flexray::FlexrayControllerConfig& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/,
                 const Services::Flexray::FlexrayTxBufferConfigUpdate& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Flexray::WireFlexrayTxBufferUpdate& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Flexray::FlexrayPocStatusEvent& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Lin::LinSendFrameRequest& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Lin::LinSendFrameHeaderRequest& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Lin::LinTransmission& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Lin::LinFrameResponseUpdate& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Lin::WireLinControllerConfig& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Lin::LinControllerStatusUpdate& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Lin::LinWakeupPulse& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const Services::PubSub::WireDataMessageEvent& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Rpc::FunctionCall& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, Services::Rpc::FunctionCall&& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Rpc::FunctionCallResponse& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, Services::Rpc::FunctionCallResponse&& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Orchestration::NextSimTask& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Orchestration::ParticipantStatus& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Orchestration::SystemCommand& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/,
                 const Services::Orchestration::WorkflowConfiguration& /*msg*/) override
    {
    }

    void SendMsg(const IServiceEndpoint* /*from*/, Services::Logging::LogMsg&& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Logging::LogMsg& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const VSilKit::MetricsUpdate& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const Discovery::ParticipantDiscoveryEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Discovery::ServiceDiscoveryEvent& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const RequestReply::RequestReplyCall& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const RequestReply::RequestReplyCallReturn& /*msg*/) override {}

    // targeted messaging

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Can::WireCanFrameEvent& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Can::CanFrameTransmitEvent& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Can::CanControllerStatus& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Can::CanConfigureBaudrate& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Can::CanSetControllerMode& /*msg*/) override
    {
    }

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Ethernet::WireEthernetFrameEvent& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Ethernet::EthernetFrameTransmitEvent& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Ethernet::EthernetStatus& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Ethernet::EthernetSetMode& /*msg*/) override
    {
    }

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Flexray::WireFlexrayFrameEvent& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Flexray::WireFlexrayFrameTransmitEvent& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Flexray::FlexraySymbolEvent& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Flexray::FlexraySymbolTransmitEvent& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Flexray::FlexrayCycleStartEvent& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Flexray::FlexrayHostCommand& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Flexray::FlexrayControllerConfig& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Flexray::FlexrayTxBufferConfigUpdate& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Flexray::WireFlexrayTxBufferUpdate& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Flexray::FlexrayPocStatusEvent& /*msg*/) override
    {
    }

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Lin::LinSendFrameRequest& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Lin::LinSendFrameHeaderRequest& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Lin::LinTransmission& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Lin::LinFrameResponseUpdate& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Lin::WireLinControllerConfig& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Lin::LinControllerStatusUpdate& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Lin::LinWakeupPulse& /*msg*/) override
    {
    }

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::PubSub::WireDataMessageEvent& /*msg*/) override
    {
    }

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Rpc::FunctionCall& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 Services::Rpc::FunctionCall&& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Rpc::FunctionCallResponse& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 Services::Rpc::FunctionCallResponse&& /*msg*/) override
    {
    }

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Orchestration::NextSimTask& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Orchestration::ParticipantStatus& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Orchestration::SystemCommand& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Orchestration::WorkflowConfiguration& /*msg*/) override
    {
    }

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 Services::Logging::LogMsg&& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Services::Logging::LogMsg& /*msg*/) override
    {
    }

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const VSilKit::MetricsUpdate& /*msg*/) override
    {
    }

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Discovery::ParticipantDiscoveryEvent& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const Discovery::ServiceDiscoveryEvent& /*msg*/) override
    {
    }

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const RequestReply::RequestReplyCall& /*msg*/) override
    {
    }
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/,
                 const RequestReply::RequestReplyCallReturn& /*msg*/) override
    {
    }


    void OnAllMessagesDelivered(std::function<void()> /*callback*/) override {}
    void FlushSendBuffers() override {}
    void ExecuteDeferred(std::function<void()> callback) override
    {
        callback();
    }

    auto GetParticipantName() const -> const std::string& override
    {
        return _name;
    }
    auto GetRegistryUri() const -> const std::string& override
    {
        return _registryUri;
    }

    virtual auto GetTimeProvider() -> Services::Orchestration::ITimeProvider*
    {
        return &mockTimeProvider;
    }
    void JoinSilKitSimulation() override {}
    void RegisterTimeSyncService(SilKit::Services::Orchestration::TimeSyncService*) override {}

    auto GetServiceDiscovery() -> Discovery::IServiceDiscovery* override
    {
        return &mockServiceDiscovery;
    }
    auto GetRequestReplyService() -> RequestReply::IRequestReplyService* override
    {
        return &mockRequestReplyService;
    }
    auto GetParticipantRepliesProcedure() -> RequestReply::IParticipantReplies* override
    {
        return &mockParticipantReplies;
    }

    void AddAsyncSubscriptionsCompletionHandler(std::function<void()> handler) override
    {
        handler();
    };

    void SetIsSystemControllerCreated(bool /*isCreated*/) override {};
    bool GetIsSystemControllerCreated() override
    {
        return false;
    };

    void SetIsNetworkSimulatorCreated(bool /*isCreated*/) override {};
    bool GetIsNetworkSimulatorCreated() override
    {
        return false;
    };

    size_t GetNumberOfConnectedParticipants() override
    {
        return 0;
    };
    auto GetMetricsManager() -> IMetricsManager* override
    {
        return &mockMetricsManager;
    }
    size_t GetNumberOfRemoteReceivers(const IServiceEndpoint* /*service*/, const std::string& /*msgTypeName*/) override
    {
        return 0;
    }

    std::vector<std::string> GetParticipantNamesOfRemoteReceivers(const IServiceEndpoint* /*service*/,
                                                                  const std::string& /*msgTypeName*/) override
    {
        return {};
    }

    void NotifyShutdown() override {};
    void EvaluateAggregationInfo(bool /*isSyncSimStepHandler*/) override {};
    void RegisterReplayController(SilKit::Tracing::IReplayDataController*, const std::string&,
                                  const SilKit::Config::SimulatedNetwork&) override
    {
    }

    bool ParticipantHasCapability(const std::string& /*participantName*/,
                                  const std::string& /*capability*/) const override
    {
        return true;
    }

    std::string GetServiceDescriptorString(
        SilKit::Experimental::NetworkSimulation::ControllerDescriptor /*controllerDescriptor*/) override
    {
        return "";
    }

    Services::Logging::ILoggerInternal* GetLoggerInternal() override
    {
        return &logger;
    }

    auto GetMetricsProcessor() -> IMetricsProcessor* override
    {
        return nullptr;
    }

    auto GetMetricsSender() -> IMetricsSender* override
    {
        return nullptr;
    }

    const std::string _name = "MockParticipant";
    const std::string _registryUri = "silkit://mock.participant.silkit:0";
    testing::NiceMock<MockLogger> logger;
    testing::NiceMock<MockTimeProvider> mockTimeProvider;
    MockLifecycleService mockLifecycleService;
    MockTimeSyncService mockTimeSyncService;
    MockSystemController mockSystemController;
    testing::NiceMock<MockSystemMonitor> mockSystemMonitor;
    testing::NiceMock<MockServiceDiscovery> mockServiceDiscovery;
    MockRequestReplyService mockRequestReplyService;
    MockParticipantReplies mockParticipantReplies;
    DummyNetworkSimulator mockNetworkSimulator;
    DummyMetricsManager mockMetricsManager;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace Tests
} // namespace Core
} // namespace SilKit
