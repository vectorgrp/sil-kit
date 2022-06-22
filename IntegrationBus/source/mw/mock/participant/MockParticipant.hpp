// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>

#include "ib/mw/sync/SyncDatatypes.hpp"
#include "ib/mw/logging/LoggingDatatypes.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/ILifecycleService.hpp"
#include "ib/mw/sync/ITimeSyncService.hpp"
#include "ib/mw/sync/ISystemController.hpp"
#include "ib/mw/sync/ISystemMonitor.hpp"

#include "ib/sim/fwd_decl.hpp"
#include "ib/sim/can/CanDatatypes.hpp"
#include "ib/sim/eth/EthernetDatatypes.hpp"
#include "ib/sim/fr/FlexrayDatatypes.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"
#include "ib/sim/data/DataMessageDatatypes.hpp"
#include "ib/sim/rpc/RpcDatatypes.hpp"

#include "TimeProvider.hpp"
#include "IParticipantInternal.hpp"
#include "IServiceDiscovery.hpp"
#include "SynchronizedHandlers.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace ib {
namespace mw {
namespace test {

class DummyLogger : public logging::ILogger
{
public:
    void Log(logging::Level /*level*/, const std::string& /*msg*/) override {}
    void Trace(const std::string& /*msg*/) override {}
    void Debug(const std::string& /*msg*/) override {}
    void Info(const std::string& /*msg*/) override {}
    void Warn(const std::string& /*msg*/) override {}
    void Error(const std::string& /*msg*/) override {}
    void Critical(const std::string& /*msg*/) override {}
    void RegisterRemoteLogging(const LogMsgHandlerT& /*handler*/) {}
    void LogReceivedMsg(const logging::LogMsg& /*msg*/) {}
protected:
    bool ShouldLog(logging::Level) const override { return true; }
};


class MockTimeProvider : public sync::ITimeProvider
{
public:
    struct MockTime
    {
        MOCK_METHOD0(Now, std::chrono::nanoseconds());
    };

    //XXX gtest 1.10 has a MOCK_METHOD macro with specifiers like const, noexcept.
    //    until then we use an auxiliary struct mockTime to get rid of "const this".
    auto Now() const -> std::chrono::nanoseconds override
    {
        return mockTime.Now();
    }
    auto TimeProviderName() const -> const std::string& override
    {
        return _name;
    }

    HandlerId AddNextSimStepHandler(NextSimStepHandlerT handler) override
    {
        return _handlers.Add(std::move(handler));
    }

    void RemoveNextSimStepHandler(HandlerId handlerId) override
    {
        _handlers.Remove(handlerId);
    }

    util::SynchronizedHandlers<NextSimStepHandlerT> _handlers;
    const std::string _name = "MockTimeProvider";
    mutable MockTime mockTime;
};


class MockLifecycleService : public sync::ILifecycleService {
public:
    MOCK_METHOD(void, SetCommunicationReadyHandler, (CommunicationReadyHandlerT), (override));
    MOCK_METHOD(void, SetStopHandler, (StopHandlerT), (override));
    MOCK_METHOD(void, SetShutdownHandler, (ShutdownHandlerT), (override));
    MOCK_METHOD(std::future<sync::ParticipantState>, ExecuteLifecycleNoSyncTime,
                (bool hasCoordinatedSimulationStart, bool hasCoordinatedSimulationStop, bool isRequiredParticipant),
                (override));
    MOCK_METHOD(std::future<sync::ParticipantState>, ExecuteLifecycleNoSyncTime,
                (bool hasCoordinatedSimulationStart, bool hasCoordinatedSimulationStop), (override));
    MOCK_METHOD(std::future<sync::ParticipantState>, ExecuteLifecycleWithSyncTime,
                (sync::ITimeSyncService * timeSyncService, bool hasCoordinatedSimulationStart,
                 bool hasCoordinatedSimulationStop, bool isRequiredParticipant),
                (override));
    MOCK_METHOD(std::future<sync::ParticipantState>, ExecuteLifecycleWithSyncTime,
                (sync::ITimeSyncService * timeSyncService, bool hasCoordinatedSimulationStart,
                 bool hasCoordinatedSimulationStop),
                (override));
    MOCK_METHOD(void, ReportError, (std::string /*errorMsg*/), (override));
    MOCK_METHOD(void, Pause, (std::string /*reason*/), (override));
    MOCK_METHOD(void, Continue, (), (override));
    MOCK_METHOD(void, Stop, (std::string /*reason*/), (override));
    MOCK_METHOD(sync::ParticipantState, State, (), (override, const));
    MOCK_METHOD(sync::ParticipantStatus&, Status, (), (override, const));
    MOCK_METHOD(sync::ITimeSyncService*, GetTimeSyncService, (), (override, const));
};

class MockTimeSyncService : public sync::ITimeSyncService
{
public:
    MOCK_METHOD(void, SetSimulationTask, (SimTaskT task), (override));
    MOCK_METHOD(void, SetSimulationTaskAsync, (SimTaskT task), (override));
    MOCK_METHOD(void, CompleteSimulationTask, (), (override));
    MOCK_METHOD(void, SetSimulationTask, (std::function<void(std::chrono::nanoseconds now)>), (override));
    MOCK_METHOD(void, SetPeriod, (std::chrono::nanoseconds period), (override));
    MOCK_METHOD(std::chrono::nanoseconds, Now, (), (override, const));
};

class MockSystemMonitor : public sync::ISystemMonitor {
public:
    MOCK_METHOD(HandlerId, AddSystemStateHandler, (SystemStateHandlerT));
    MOCK_METHOD(void, RemoveSystemStateHandler, (HandlerId));

    MOCK_METHOD(HandlerId, AddParticipantStatusHandler, (ParticipantStatusHandlerT));
    MOCK_METHOD(void, RemoveParticipantStatusHandler, (HandlerId));

    MOCK_CONST_METHOD0(SystemState,  sync::SystemState());
    MOCK_CONST_METHOD1(ParticipantStatus, const sync::ParticipantStatus&(const std::string& participantName));

    MOCK_METHOD(void, SetParticipantConnectedHandler, (ParticipantConnectedHandler handler), (override));
    MOCK_METHOD(void, SetParticipantDisconnectedHandler, (ParticipantDisconnectedHandler handler), (override));
    MOCK_METHOD(bool, IsParticipantConnected, (const std::string& participantName), (const, override));
};

class MockSystemController : public sync::ISystemController {
public:
    MOCK_METHOD(void, Initialize, (const std::string& participantId), (const, override));
    MOCK_METHOD(void, Restart, (const std::string& participantId), (const, override));
    MOCK_CONST_METHOD0(Run, void());
    MOCK_CONST_METHOD0(Stop, void());
    MOCK_CONST_METHOD0(AbortSimulation, void());
    MOCK_CONST_METHOD0(Shutdown, void());
    MOCK_METHOD((void), SetRequiredParticipants, (const std::vector<std::string>& participantNames));
};

class MockServiceDiscovery : public service::IServiceDiscovery
{
public:
    MOCK_METHOD(void, NotifyServiceCreated, (const ServiceDescriptor& serviceDescriptor), (override));
    MOCK_METHOD(void, NotifyServiceRemoved, (const ServiceDescriptor& serviceDescriptor), (override));
    MOCK_METHOD(void, RegisterServiceDiscoveryHandler, (ib::mw::service::ServiceDiscoveryHandlerT handler), (override));
    MOCK_METHOD(void, RegisterSpecificServiceDiscoveryHandler,
                (ib::mw::service::ServiceDiscoveryHandlerT handler, const std::string& controllerTypeName,
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
        -> sim::can::ICanController* override
    {
        return nullptr;
    }
    auto CreateCanController(const std::string & /*canonicalName*/) -> sim::can::ICanController* override
    {
        return nullptr;
    }
    auto CreateEthernetController(const std::string & /*canonicalName*/, const std::string& /*networkName*/) -> sim::eth::IEthernetController* override
    {
        return nullptr;
    }
    auto CreateEthernetController(const std::string & /*canonicalName*/) -> sim::eth::IEthernetController* override
    {
        return nullptr;
    }
    auto CreateFlexrayController(const std::string& /*canonicalName*/) -> sim::fr::IFlexrayController* override
    {
        return nullptr;
    }
    auto CreateFlexrayController(const std::string& /*canonicalName*/, const std::string & /*networkName*/)
        -> sim::fr::IFlexrayController* override
    {
        return nullptr;
    }
    auto CreateLinController(const std::string& /*canonicalName*/, const std::string & /*networkName*/)
        -> sim::lin::ILinController* override
    {
        return nullptr;
    }
    auto CreateLinController(const std::string & /*canonicalName*/) -> sim::lin::ILinController* override
    {
        return nullptr;
    }
    auto CreateDataPublisher(const std::string& /*controllerName*/, const std::string& /*topic*/,
                             const std::string& /*mediaType*/, const std::map<std::string, std::string>& /*labels*/,
                             size_t /* history */) -> ib::sim::data::IDataPublisher* override
    {
        return nullptr;
    }
    auto CreateDataPublisher(const std::string& /*controllerName*/) -> ib::sim::data::IDataPublisher* override
    {
        return nullptr;
    }
    auto CreateDataSubscriber(const std::string& /*controllerName*/, const std::string& /*topic*/,
                              const std::string& /*mediaType*/, const std::map<std::string, std::string>& /*labels*/,
                              ib::sim::data::DataMessageHandlerT /* callback*/,
                              ib::sim::data::NewDataPublisherHandlerT /*newDataSourceHandler*/)
        -> ib::sim::data::IDataSubscriber* override
    {
        return nullptr;
    }
    auto CreateDataSubscriber(const std::string& /*controllerName*/) -> ib::sim::data::IDataSubscriber* override
    {
        return nullptr;
    }
    auto CreateDataSubscriberInternal(const std::string& /*topic*/, const std::string& /*linkName*/,
                                      const std::string& /*mediaType*/,
                                      const std::map<std::string, std::string>& /*publisherLabels*/,
                                      sim::data::DataMessageHandlerT /*callback*/,
                                      sim::data::IDataSubscriber* /*parent*/)
        -> sim::data::DataSubscriberInternal* override
    {
        return nullptr;
    }

    auto CreateRpcClient(const std::string& /*controllerName*/, const std::string& /*functionName*/,
                         const std::string& /*mediaType*/, const std::map<std::string, std::string>& /*labels*/,
                         ib::sim::rpc::RpcCallResultHandler /*handler*/) -> ib::sim::rpc::IRpcClient* override
    {
        return nullptr;
    }
    auto CreateRpcClient(const std::string& /*controllerName*/) -> ib::sim::rpc::IRpcClient* override
    {
        return nullptr;
    }
    auto CreateRpcServer(const std::string& /*controllerName*/, const std::string& /*functionName*/,
                         const std::string& /*mediaType*/, const std::map<std::string, std::string>& /*labels*/,
                         ib::sim::rpc::RpcCallHandler /*handler*/) -> ib::sim::rpc::IRpcServer* override
    {
        return nullptr;
    }
    auto CreateRpcServer(const std::string& /*controllerName*/) -> ib::sim::rpc::IRpcServer* override
    {
        return nullptr;
    }
    auto CreateRpcServerInternal(const std::string& /*functionName*/, const std::string& /*linkName*/,
                                 const std::string& /*mediaType*/, const std::map<std::string, std::string>& /*labels*/,
                                 sim::rpc::RpcCallHandler /*handler*/, sim::rpc::IRpcServer* /*parent*/)
        -> ib::sim::rpc::RpcServerInternal* override
    {
        return nullptr;
    }

    void DiscoverRpcServers(const std::string& /*functionName*/, const std::string& /*mediaType*/,
                            const std::map<std::string, std::string>& /*labels*/,
                            sim::rpc::RpcDiscoveryResultHandler /*handler*/) override{};

    auto GetLifecycleService() -> sync::ILifecycleService* override { return &mockLifecycleService; }
    // TODO mock this?
    auto CreateTimeSyncService(sync::LifecycleService*) -> sync::TimeSyncService* override { return nullptr; };
    auto GetSystemMonitor() -> sync::ISystemMonitor* override { return &mockSystemMonitor; }
    auto GetSystemController() -> sync::ISystemController* override { return &mockSystemController; }

    auto GetLogger() -> logging::ILogger* override { return &logger; }

    void RegisterCanSimulator(sim::can::IIbToCanSimulator*, const std::vector<std::string>& ) override {}
    void RegisterEthSimulator(sim::eth::IIbToEthSimulator* , const std::vector<std::string>&) override {}
    void RegisterFlexraySimulator(sim::fr::IIbToFlexrayBusSimulator* , const std::vector<std::string>&) override {}
    void RegisterLinSimulator(sim::lin::IIbToLinSimulator*, const std::vector<std::string>&) override {}

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, sim::can::CanFrameEvent&& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::can::CanFrameEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::can::CanFrameTransmitEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::can::CanControllerStatus& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::can::CanConfigureBaudrate& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::can::CanSetControllerMode& /*msg*/) override {}

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, sim::eth::EthernetFrameEvent&& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::eth::EthernetFrameEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::eth::EthernetFrameTransmitEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::eth::EthernetStatus& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::eth::EthernetSetMode& /*msg*/) override {}

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, sim::fr::FlexrayFrameEvent&& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::fr::FlexrayFrameEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, sim::fr::FlexrayFrameTransmitEvent&& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::fr::FlexrayFrameTransmitEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::fr::FlexraySymbolEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::fr::FlexraySymbolTransmitEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::fr::FlexrayCycleStartEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::fr::FlexrayHostCommand& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::fr::FlexrayControllerConfig& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::fr::FlexrayTxBufferConfigUpdate& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::fr::FlexrayTxBufferUpdate& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::fr::FlexrayPocStatusEvent& /*msg*/) override {}

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::lin::LinSendFrameRequest& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::lin::LinSendFrameHeaderRequest& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::lin::LinTransmission& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::lin::LinFrameResponseUpdate& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::lin::LinControllerConfig& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::lin::LinControllerStatusUpdate& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::lin::LinWakeupPulse& /*msg*/) override {}

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, sim::data::DataMessageEvent&& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::data::DataMessageEvent& /*msg*/) override {}

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::rpc::FunctionCall& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, sim::rpc::FunctionCall&& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sim::rpc::FunctionCallResponse& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, sim::rpc::FunctionCallResponse&& /*msg*/) override {}

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sync::NextSimTask& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sync::ParticipantStatus& /*msg*/)  override{}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sync::ParticipantCommand& /*msg*/)  override{}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sync::SystemCommand& /*msg*/)  override{}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const sync::ExpectedParticipants& /*msg*/)  override{}

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, logging::LogMsg&& /*msg*/)  override{}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const logging::LogMsg& /*msg*/)  override{}

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const service::ParticipantDiscoveryEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const service::ServiceDiscoveryEvent& /*msg*/)  override{}

    // targeted messaging

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, sim::can::CanFrameEvent&& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::can::CanFrameEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::can::CanFrameTransmitEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::can::CanControllerStatus& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::can::CanConfigureBaudrate& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::can::CanSetControllerMode& /*msg*/) override {}

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, sim::eth::EthernetFrameEvent&& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::eth::EthernetFrameEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::eth::EthernetFrameTransmitEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::eth::EthernetStatus& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::eth::EthernetSetMode& /*msg*/) override {}

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, sim::fr::FlexrayFrameEvent&& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::fr::FlexrayFrameEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, sim::fr::FlexrayFrameTransmitEvent&& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::fr::FlexrayFrameTransmitEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::fr::FlexraySymbolEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::fr::FlexraySymbolTransmitEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::fr::FlexrayCycleStartEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::fr::FlexrayHostCommand& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::fr::FlexrayControllerConfig& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::fr::FlexrayTxBufferConfigUpdate& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::fr::FlexrayTxBufferUpdate& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::fr::FlexrayPocStatusEvent& /*msg*/) override {}

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::lin::LinSendFrameRequest& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::lin::LinSendFrameHeaderRequest& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::lin::LinTransmission& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::lin::LinFrameResponseUpdate& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::lin::LinControllerConfig& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::lin::LinControllerStatusUpdate& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::lin::LinWakeupPulse& /*msg*/) override {}

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::data::DataMessageEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, sim::data::DataMessageEvent&& /*msg*/) override {}

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::rpc::FunctionCall& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, sim::rpc::FunctionCall&& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sim::rpc::FunctionCallResponse& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, sim::rpc::FunctionCallResponse&& /*msg*/) override {}

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sync::NextSimTask& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sync::ParticipantStatus& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sync::ParticipantCommand& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sync::SystemCommand& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const sync::ExpectedParticipants& /*msg*/) override {}

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, logging::LogMsg&& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const logging::LogMsg& /*msg*/) override {}

    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const service::ParticipantDiscoveryEvent& /*msg*/) override {}
    void SendIbMessage(const IIbServiceEndpoint* /*from*/, const std::string& /*targetParticipantName*/, const service::ServiceDiscoveryEvent& /*msg*/) override {}


    void OnAllMessagesDelivered(std::function<void()> /*callback*/) override {}
    void FlushSendBuffers() override {}
    void ExecuteDeferred(std::function<void()> /*callback*/) override {}
    auto GetParticipantName() const -> const std::string& override { return _name; }

    virtual auto GetTimeProvider() -> sync::ITimeProvider* { return &mockTimeProvider; }
    void joinIbDomain(uint32_t ) override {}

    auto GetServiceDiscovery() -> service::IServiceDiscovery* override { return &mockServiceDiscovery; }

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

} // namespace test
} // namespace mw
} // namespace ib
