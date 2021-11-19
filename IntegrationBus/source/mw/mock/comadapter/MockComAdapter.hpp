// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>

#include "ib/mw/sync/SyncDatatypes.hpp"
#include "ib/mw/logging/LoggingDatatypes.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/IParticipantController.hpp"

#include "ib/sim/fwd_decl.hpp"
#include "ib/sim/can/CanDatatypes.hpp"
#include "ib/sim/eth/EthDatatypes.hpp"
#include "ib/sim/fr/FrDatatypes.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"
#include "ib/sim/io/IoDatatypes.hpp"
#include "ib/sim/generic/GenericMessageDatatypes.hpp"

#include "TimeProvider.hpp"
#include "IComAdapterInternal.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


#ifdef SendMessage
#undef SendMessage
#endif

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

    void RegisterNextSimStepHandler(NextSimStepHandlerT handler)
    {
        _handlers.emplace_back(std::move(handler));
    }

    std::vector<NextSimStepHandlerT> _handlers;
    const std::string _name = "MockTimeProvider";
    mutable MockTime mockTime;
};


class MockParticipantController : public sync::IParticipantController {
public:
    MOCK_METHOD1(SetInitHandler, void(InitHandlerT));
    MOCK_METHOD1(SetStopHandler, void(StopHandlerT));
    MOCK_METHOD1(SetShutdownHandler, void(ShutdownHandlerT));
    MOCK_METHOD1(SetSimulationTask, void(SimTaskT task));
    MOCK_METHOD1(SetSimulationTask, void(std::function<void(std::chrono::nanoseconds now)>));
    MOCK_METHOD0(EnableColdswap, void());
    MOCK_METHOD1(SetPeriod, void(std::chrono::nanoseconds period));
    MOCK_METHOD1(SetEarliestEventTime, void(std::chrono::nanoseconds eventTime));
    MOCK_METHOD0(Run, sync::ParticipantState());
    MOCK_METHOD0(RunAsync, std::future<sync::ParticipantState>());
    MOCK_METHOD1(ReportError, void(std::string errorMsg));
    MOCK_METHOD1(Pause, void(std::string reason));
    MOCK_METHOD0(Continue, void());
    MOCK_METHOD1(Stop, void(std::string reason));
    MOCK_CONST_METHOD0(State,  sync::ParticipantState());
    MOCK_CONST_METHOD0(Status, sync::ParticipantStatus&());
    MOCK_METHOD0(RefreshStatus, void());
    MOCK_CONST_METHOD0(Now, std::chrono::nanoseconds());
    MOCK_METHOD0(LogCurrentPerformanceStats, void());
    MOCK_METHOD1(ForceShutdown, void(std::string reason));
};

class DummyComAdapter : public IComAdapterInternal
{
public:
    DummyComAdapter()
    {
    }

    auto CreateCanController(const std::string& /*canonicalName*/) -> sim::can::ICanController* { return nullptr; }
    auto CreateEthController(const std::string& /*canonicalName*/) -> sim::eth::IEthController* { return nullptr; }
    auto CreateFlexrayController(const std::string& /*canonicalName*/) -> sim::fr::IFrController* { return nullptr; }
    auto CreateLinController(const std::string& /*canonicalName*/) -> sim::lin::ILinController* { return nullptr; }
    auto CreateAnalogIn(const std::string& /*canonicalName*/) -> sim::io::IAnalogInPort* { return nullptr; }
    auto CreateDigitalIn(const std::string& /*canonicalName*/) -> sim::io::IDigitalInPort* { return nullptr; }
    auto CreatePwmIn(const std::string& /*canonicalName*/) -> sim::io::IPwmInPort* { return nullptr; }
    auto CreatePatternIn(const std::string& /*canonicalName*/) -> sim::io::IPatternInPort* { return nullptr; }
    auto CreateAnalogOut(const std::string& /*canonicalName*/) -> sim::io::IAnalogOutPort* { return nullptr; }
    auto CreateDigitalOut(const std::string& /*canonicalName*/) -> sim::io::IDigitalOutPort* { return nullptr; }
    auto CreatePwmOut(const std::string& /*canonicalName*/) -> sim::io::IPwmOutPort* { return nullptr; }
    auto CreatePatternOut(const std::string& /*canonicalName*/) -> sim::io::IPatternOutPort* { return nullptr; }
    auto CreateGenericPublisher(const std::string& /*canonicalName*/) -> sim::generic::IGenericPublisher* { return nullptr; }
    auto CreateGenericSubscriber(const std::string& /*canonicalName*/) -> sim::generic::IGenericSubscriber* { return nullptr; }

    auto GetSyncMaster() -> sync::ISyncMaster* { return nullptr; }
    
    auto GetParticipantController() -> sync::IParticipantController* { return &mockParticipantController; }

    auto GetSystemMonitor() -> sync::ISystemMonitor* { return nullptr; }
    auto GetSystemController() -> sync::ISystemController* { return nullptr; }
    auto GetLogger() -> logging::ILogger* { return &logger; }

    virtual auto GetTimeProvider() -> sync::ITimeProvider* { return &mockTimeProvider; }
    void joinIbDomain(uint32_t ) override {}
    void RegisterCanSimulator(sim::can::IIbToCanSimulator* ) override {}
    void RegisterEthSimulator(sim::eth::IIbToEthSimulator* ) override {}
    void RegisterFlexraySimulator(sim::fr::IIbToFrBusSimulator* ) override {}
    void RegisterLinSimulator(sim::lin::IIbToLinSimulator*) override {}

    void SendIbMessage(const IServiceId* /*from*/, sim::can::CanMessage&& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::can::CanMessage& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::can::CanTransmitAcknowledge& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::can::CanControllerStatus& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::can::CanConfigureBaudrate& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::can::CanSetControllerMode& /*msg*/) {}
                        
    void SendIbMessage(const IServiceId* /*from*/, sim::eth::EthMessage&& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::eth::EthMessage& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::eth::EthTransmitAcknowledge& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::eth::EthStatus& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::eth::EthSetMode& /*msg*/) {}
                        
    void SendIbMessage(const IServiceId* /*from*/, sim::fr::FrMessage&& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::fr::FrMessage& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, sim::fr::FrMessageAck&& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::fr::FrMessageAck& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::fr::FrSymbol& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::fr::FrSymbolAck& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::fr::CycleStart& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::fr::HostCommand& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::fr::ControllerConfig& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::fr::TxBufferConfigUpdate& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::fr::TxBufferUpdate& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::fr::ControllerStatus& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::fr::PocStatus& /*msg*/) {}
                        
    void SendIbMessage(const IServiceId* /*from*/, const sim::lin::SendFrameRequest& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::lin::SendFrameHeaderRequest& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::lin::Transmission& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::lin::FrameResponseUpdate& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::lin::ControllerConfig& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::lin::ControllerStatusUpdate& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::lin::WakeupPulse& /*msg*/) {}
                        
    void SendIbMessage(const IServiceId* /*from*/, const sim::io::AnalogIoMessage& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::io::DigitalIoMessage& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, sim::io::PatternIoMessage&& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::io::PatternIoMessage& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::io::PwmIoMessage& /*msg*/) {}
                        
    void SendIbMessage(const IServiceId* /*from*/, sim::generic::GenericMessage&& /*msg*/) {}
    void SendIbMessage(const IServiceId* /*from*/, const sim::generic::GenericMessage& /*msg*/) {}
    virtual void SendIbMessage_proxy(const IServiceId* /*from*/, const sim::generic::GenericMessage& /*msg*/) {}

    void SendIbMessage(const ib::mw::IServiceId* /*from*/, const sync::NextSimTask& /*msg*/) {}
    void SendIbMessage(const ib::mw::IServiceId* /*from*/, const sync::Tick& /*msg*/) {}
    void SendIbMessage(const ib::mw::IServiceId* /*from*/, const sync::TickDone& /*msg*/) {}
    void SendIbMessage(const ib::mw::IServiceId* /*from*/, const sync::QuantumRequest& /*msg*/) {}
    void SendIbMessage(const ib::mw::IServiceId* /*from*/, const sync::QuantumGrant& /*msg*/) {}
    void SendIbMessage(const ib::mw::IServiceId* /*from*/, const sync::ParticipantStatus& /*msg*/) {}
    void SendIbMessage(const ib::mw::IServiceId* /*from*/, const sync::ParticipantCommand& /*msg*/) {}
    void SendIbMessage(const ib::mw::IServiceId* /*from*/, const sync::SystemCommand& /*msg*/) {}
                        
    void SendIbMessage(const ib::mw::IServiceId* /*from*/, logging::LogMsg&& /*msg*/) {}
    void SendIbMessage(const ib::mw::IServiceId* /*from*/, const logging::LogMsg& /*msg*/) {}

    void OnAllMessagesDelivered(std::function<void(void)> /*callback*/) {}
    void FlushSendBuffers() {}
    auto GetParticipantName() const -> const std::string& override { throw std::runtime_error("invalid call"); }
    auto GetConfig() const -> const ib::cfg::Config& override { throw std::runtime_error("invalid call"); }

    DummyLogger logger;
    MockTimeProvider mockTimeProvider;
    MockParticipantController mockParticipantController;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace test
} // namespace mw
} // namespace ib
