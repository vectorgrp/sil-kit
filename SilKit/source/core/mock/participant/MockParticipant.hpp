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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/services/orchestration/SyncDatatypes.hpp"
#include "silkit/services/logging/LoggingDatatypes.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/orchestration/ILifecycleService.hpp"
#include "silkit/services/orchestration/ITimeSyncService.hpp"
#include "silkit/services/orchestration/ISystemController.hpp"
#include "silkit/services/orchestration/ISystemMonitor.hpp"

#include "silkit/services/fwd_decl.hpp"
#include "silkit/services/can/CanDatatypes.hpp"
#include "silkit/services/ethernet/EthernetDatatypes.hpp"
#include "silkit/services/flexray/FlexrayDatatypes.hpp"
#include "silkit/services/lin/LinDatatypes.hpp"
#include "silkit/services/pubsub/DataMessageDatatypes.hpp"
#include "silkit/services/rpc/RpcDatatypes.hpp"

#include "IParticipantInternal.hpp"
#include "IServiceDiscovery.hpp"
#include "LifecycleService.hpp"
#include "SynchronizedHandlers.hpp"
#include "MockTimeProvider.hpp"

namespace SilKit {
namespace Core {
namespace Tests {

using SilKit::Util::HandlerId;

class DummyLogger : public Services::Logging::ILogger
{
public:
    void Log(Services::Logging::Level /*level*/, const std::string& /*msg*/) override {}
    void Trace(const std::string& /*msg*/) override {}
    void Debug(const std::string& /*msg*/) override {}
    void Info(const std::string& /*msg*/) override {}
    void Warn(const std::string& /*msg*/) override {}
    void Error(const std::string& /*msg*/) override {}
    void Critical(const std::string& /*msg*/) override {}
    void RegisterRemoteLogging(const LogMsgHandlerT& /*handler*/) {}
    void LogReceivedMsg(const Services::Logging::LogMsg& /*msg*/) {}
    Services::Logging::Level GetLogLevel() const override { return Services::Logging::Level::Debug; }
};

class MockLifecycleService
    : public Services::Orchestration::ILifecycleServiceInternal
{
public:
    MOCK_METHOD(void, SetCommunicationReadyHandler, (SilKit::Services::Orchestration::CommunicationReadyHandler), (override));
    MOCK_METHOD(void, SetStartingHandler, (SilKit::Services::Orchestration::StartingHandler), (override));
    MOCK_METHOD(void, SetStopHandler, (SilKit::Services::Orchestration::StopHandler), (override));
    MOCK_METHOD(void, SetShutdownHandler, (SilKit::Services::Orchestration::ShutdownHandler), (override));
    MOCK_METHOD(std::future<Services::Orchestration::ParticipantState>, StartLifecycle,
                (Services::Orchestration::LifecycleConfiguration), (override));
    MOCK_METHOD(void, ReportError, (std::string /*errorMsg*/), (override));
    MOCK_METHOD(void, Pause, (std::string /*reason*/), (override));
    MOCK_METHOD(void, Continue, (), (override));
    MOCK_METHOD(void, Stop, (std::string /*reason*/), (override));
    MOCK_METHOD(Services::Orchestration::ParticipantState, State, (), (override, const));
    MOCK_METHOD(Services::Orchestration::ParticipantStatus&, Status, (), (override, const));
    MOCK_METHOD(Services::Orchestration::ITimeSyncService*, GetTimeSyncService, (), (override, const));
    MOCK_METHOD(void, SetTimeSyncActive, (bool /*isTimeSyncActive*/), (override));
};

class MockTimeSyncService : public Services::Orchestration::ITimeSyncService
{
public:
    MOCK_METHOD(void, SetSimulationStepHandler, (SimulationStepT task, std::chrono::nanoseconds initialStepSize), (override));
    MOCK_METHOD(void, SetSimulationStepHandlerAsync, (SimulationStepT task, std::chrono::nanoseconds initialStepSize),
                (override));
    MOCK_METHOD(void, CompleteSimulationStep, (), (override));
    MOCK_METHOD(void, SetSimulationStepHandler,
                (std::function<void(std::chrono::nanoseconds now)>, std::chrono::nanoseconds initialStepSize),
                (override));
    MOCK_METHOD(std::chrono::nanoseconds, Now, (), (override, const));
};

class MockSystemMonitor : public Services::Orchestration::ISystemMonitor {
public:
    MOCK_METHOD(HandlerId, AddSystemStateHandler, (SystemStateHandlerT));
    MOCK_METHOD(void, RemoveSystemStateHandler, (HandlerId));

    MOCK_METHOD(HandlerId, AddParticipantStatusHandler, (ParticipantStatusHandlerT));
    MOCK_METHOD(void, RemoveParticipantStatusHandler, (HandlerId));

    MOCK_CONST_METHOD0(SystemState,  Services::Orchestration::SystemState());
    MOCK_CONST_METHOD1(ParticipantStatus, const Services::Orchestration::ParticipantStatus&(const std::string& participantName));

    MOCK_METHOD(void, SetParticipantConnectedHandler, (ParticipantConnectedHandler handler), (override));
    MOCK_METHOD(void, SetParticipantDisconnectedHandler, (ParticipantDisconnectedHandler handler), (override));
    MOCK_METHOD(bool, IsParticipantConnected, (const std::string& participantName), (const, override));
};

class MockSystemController : public Services::Orchestration::ISystemController {
public:
    MOCK_METHOD(void, Restart, (const std::string& participantId), (const, override));
    MOCK_CONST_METHOD0(Run, void());
    MOCK_CONST_METHOD0(Stop, void());
    MOCK_CONST_METHOD0(AbortSimulation, void());
    MOCK_METHOD(void, Shutdown, (const std::string&), (const, override));
    MOCK_METHOD((void), SetWorkflowConfiguration, (const SilKit::Services::Orchestration::WorkflowConfiguration& workflowConfiguration));
};

class MockServiceDiscovery : public Discovery::IServiceDiscovery
{
public:
    MOCK_METHOD(void, NotifyServiceCreated, (const ServiceDescriptor& serviceDescriptor), (override));
    MOCK_METHOD(void, NotifyServiceRemoved, (const ServiceDescriptor& serviceDescriptor), (override));
    MOCK_METHOD(void, RegisterServiceDiscoveryHandler, (SilKit::Core::Discovery::ServiceDiscoveryHandlerT handler), (override));
    MOCK_METHOD(void, RegisterSpecificServiceDiscoveryHandler,
                (SilKit::Core::Discovery::ServiceDiscoveryHandlerT handler, const std::string& controllerTypeName,
                 const std::string& supplDataValue),
                (override));
    MOCK_METHOD(std::vector<ServiceDescriptor>, GetServices, (), (const, override));
    MOCK_METHOD(void, OnParticpantRemoval, (const std::string& participantName), (override));

};

class DummyParticipant : public IParticipantInternal
{
public:
    DummyParticipant()
    {
        ON_CALL(mockLifecycleService, GetTimeSyncService)
            .WillByDefault(testing::Return(&mockTimeSyncService));
    }

    auto CreateCanController(const std::string& /*canonicalName*/, const std::string & /*networkName*/)
        -> Services::Can::ICanController* override
    {
        return nullptr;
    }
    auto CreateEthernetController(const std::string & /*canonicalName*/, const std::string& /*networkName*/) -> Services::Ethernet::IEthernetController* override
    {
        return nullptr;
    }
    auto CreateFlexrayController(const std::string& /*canonicalName*/, const std::string & /*networkName*/)
        -> Services::Flexray::IFlexrayController* override
    {
        return nullptr;
    }
    auto CreateLinController(const std::string& /*canonicalName*/, const std::string & /*networkName*/)
        -> Services::Lin::ILinController* override
    {
        return nullptr;
    }
    auto CreateDataPublisher(const std::string& /*controllerName*/, const std::string& /*topic*/,
                             const std::string& /*mediaType*/, const std::map<std::string, std::string>& /*labels*/,
                             size_t /* history */) -> SilKit::Services::PubSub::IDataPublisher* override
    {
        return nullptr;
    }
    auto CreateDataSubscriber(const std::string& /*controllerName*/, const std::string& /*topic*/,
                              const std::string& /*mediaType*/, const std::map<std::string, std::string>& /*labels*/,
                              SilKit::Services::PubSub::DataMessageHandlerT /* callback*/,
                              SilKit::Services::PubSub::NewDataPublisherHandlerT /*newDataSourceHandler*/)
        -> SilKit::Services::PubSub::IDataSubscriber* override
    {
        return nullptr;
    }
    auto CreateDataSubscriberInternal(const std::string& /*topic*/, const std::string& /*linkName*/,
                                      const std::string& /*mediaType*/,
                                      const std::map<std::string, std::string>& /*publisherLabels*/,
                                      Services::PubSub::DataMessageHandlerT /*callback*/,
                                      Services::PubSub::IDataSubscriber* /*parent*/)
        -> Services::PubSub::DataSubscriberInternal* override
    {
        return nullptr;
    }

    auto CreateRpcClient(const std::string& /*controllerName*/, const std::string& /*functionName*/,
                         const std::string& /*mediaType*/, const std::map<std::string, std::string>& /*labels*/,
                         SilKit::Services::Rpc::RpcCallResultHandler /*handler*/) -> SilKit::Services::Rpc::IRpcClient* override
    {
        return nullptr;
    }
    auto CreateRpcServer(const std::string& /*controllerName*/, const std::string& /*functionName*/,
                         const std::string& /*mediaType*/, const std::map<std::string, std::string>& /*labels*/,
                         SilKit::Services::Rpc::RpcCallHandler /*handler*/) -> SilKit::Services::Rpc::IRpcServer* override
    {
        return nullptr;
    }
    auto CreateRpcServerInternal(const std::string& /*functionName*/, const std::string& /*linkName*/,
                                 const std::string& /*mediaType*/, const std::map<std::string, std::string>& /*labels*/,
                                 Services::Rpc::RpcCallHandler /*handler*/, Services::Rpc::IRpcServer* /*parent*/)
        -> SilKit::Services::Rpc::RpcServerInternal* override
    {
        return nullptr;
    }

    void DiscoverRpcServers(const std::string& /*functionName*/, const std::string& /*mediaType*/,
                            const std::map<std::string, std::string>& /*labels*/,
                            Services::Rpc::RpcDiscoveryResultHandler /*handler*/) override{};

    auto GetLifecycleService() -> Services::Orchestration::ILifecycleServiceInternal* override 
    {
        return &mockLifecycleService;
    }
    auto CreateLifecycleServiceNoTimeSync() -> Services::Orchestration::ILifecycleServiceNoTimeSync* override { return &mockLifecycleService; }
    auto CreateLifecycleServiceWithTimeSync() -> Services::Orchestration::ILifecycleServiceWithTimeSync* override { return &mockLifecycleService; }

    MOCK_METHOD(Services::Orchestration::TimeSyncService*, CreateTimeSyncService, (Services::Orchestration::LifecycleService*), (override));
    auto GetSystemMonitor() -> Services::Orchestration::ISystemMonitor* override { return &mockSystemMonitor; }
    auto CreateSystemMonitor() -> Services::Orchestration::ISystemMonitor* override { return &mockSystemMonitor; }
    auto GetSystemController() -> Services::Orchestration::ISystemController* override { return &mockSystemController; }
    auto CreateSystemController() -> Services::Orchestration::ISystemController* override { return &mockSystemController; }

    auto GetLogger() -> Services::Logging::ILogger* override { return &logger; }
    auto CreateLogger() -> Services::Logging::ILogger* override { return &logger; }

    void RegisterCanSimulator(Services::Can::IMsgForCanSimulator*, const std::vector<std::string>& ) override {}
    void RegisterEthSimulator(Services::Ethernet::IMsgForEthSimulator* , const std::vector<std::string>&) override {}
    void RegisterFlexraySimulator(Services::Flexray::IMsgForFlexrayBusSimulator* , const std::vector<std::string>&) override {}
    void RegisterLinSimulator(Services::Lin::IMsgForLinSimulator*, const std::vector<std::string>&) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Can::WireCanFrameEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Can::CanFrameTransmitEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Can::CanControllerStatus& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Can::CanConfigureBaudrate& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Can::CanSetControllerMode& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Ethernet::WireEthernetFrameEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Ethernet::EthernetFrameTransmitEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Ethernet::EthernetStatus& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Ethernet::EthernetSetMode& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Flexray::WireFlexrayFrameEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Flexray::WireFlexrayFrameTransmitEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Flexray::FlexraySymbolEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Flexray::FlexraySymbolTransmitEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Flexray::FlexrayCycleStartEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Flexray::FlexrayHostCommand& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Flexray::FlexrayControllerConfig& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Flexray::FlexrayTxBufferConfigUpdate& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Flexray::WireFlexrayTxBufferUpdate& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Flexray::FlexrayPocStatusEvent& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Lin::LinSendFrameRequest& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Lin::LinSendFrameHeaderRequest& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Lin::LinTransmission& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Lin::LinFrameResponseUpdate& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Lin::LinControllerConfig& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Lin::LinControllerStatusUpdate& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Lin::LinWakeupPulse& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const Services::PubSub::WireDataMessageEvent& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Rpc::FunctionCall& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, Services::Rpc::FunctionCall&& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Rpc::FunctionCallResponse& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, Services::Rpc::FunctionCallResponse&& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Orchestration::NextSimTask& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Orchestration::ParticipantStatus& /*msg*/)  override{}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Orchestration::ParticipantCommand& /*msg*/)  override{}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Orchestration::SystemCommand& /*msg*/)  override{}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Orchestration::WorkflowConfiguration& /*msg*/)  override{}

    void SendMsg(const IServiceEndpoint* /*from*/, Services::Logging::LogMsg&& /*msg*/)  override{}
    void SendMsg(const IServiceEndpoint* /*from*/, const Services::Logging::LogMsg& /*msg*/)  override{}

    void SendMsg(const IServiceEndpoint* /*from*/, const Discovery::ParticipantDiscoveryEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const Discovery::ServiceDiscoveryEvent& /*msg*/)  override{}

    // targeted messaging

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Can::WireCanFrameEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Can::CanFrameTransmitEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Can::CanControllerStatus& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Can::CanConfigureBaudrate& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Can::CanSetControllerMode& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Ethernet::WireEthernetFrameEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Ethernet::EthernetFrameTransmitEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Ethernet::EthernetStatus& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Ethernet::EthernetSetMode& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Flexray::WireFlexrayFrameEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Flexray::WireFlexrayFrameTransmitEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Flexray::FlexraySymbolEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Flexray::FlexraySymbolTransmitEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Flexray::FlexrayCycleStartEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Flexray::FlexrayHostCommand& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Flexray::FlexrayControllerConfig& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Flexray::FlexrayTxBufferConfigUpdate& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Flexray::WireFlexrayTxBufferUpdate& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Flexray::FlexrayPocStatusEvent& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Lin::LinSendFrameRequest& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Lin::LinSendFrameHeaderRequest& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Lin::LinTransmission& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Lin::LinFrameResponseUpdate& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Lin::LinControllerConfig& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Lin::LinControllerStatusUpdate& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Lin::LinWakeupPulse& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::PubSub::WireDataMessageEvent& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Rpc::FunctionCall& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, Services::Rpc::FunctionCall&& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Rpc::FunctionCallResponse& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, Services::Rpc::FunctionCallResponse&& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Orchestration::NextSimTask& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Orchestration::ParticipantStatus& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Orchestration::ParticipantCommand& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Orchestration::SystemCommand& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Orchestration::WorkflowConfiguration& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, Services::Logging::LogMsg&& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Services::Logging::LogMsg& /*msg*/) override {}

    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Discovery::ParticipantDiscoveryEvent& /*msg*/) override {}
    void SendMsg(const IServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const Discovery::ServiceDiscoveryEvent& /*msg*/) override {}


    void OnAllMessagesDelivered(std::function<void()> /*callback*/) override {}
    void FlushSendBuffers() override {}
    void ExecuteDeferred(std::function<void()> callback) override
    {
        callback();
    }
    auto GetParticipantName() const -> const std::string& override { return _name; }

    virtual auto GetTimeProvider() -> Services::Orchestration::ITimeProvider* { return &mockTimeProvider; }
    void JoinSilKitSimulation(const std::string& ) override {}

    auto GetServiceDiscovery() -> Discovery::IServiceDiscovery* override { return &mockServiceDiscovery; }

    const std::string _name = "MockParticipant";
    DummyLogger logger;
    MockTimeProvider mockTimeProvider;
    MockLifecycleService mockLifecycleService;
    MockTimeSyncService mockTimeSyncService;
    MockSystemController mockSystemController;
    MockSystemMonitor mockSystemMonitor;
    MockServiceDiscovery mockServiceDiscovery;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace Tests
} // namespace Core
} // namespace SilKit
