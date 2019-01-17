// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IComAdapter.hpp"
#include "ib/mw/sync/SyncDatatypes.hpp"

#include "ib/sim/fwd_decl.hpp"
#include "ib/sim/can/CanDatatypes.hpp"
#include "ib/sim/eth/EthDatatypes.hpp"
#include "ib/sim/fr/FrDatatypes.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"
#include "ib/sim/io/IoDatatypes.hpp"
#include "ib/sim/generic/GenericMessageDatatypes.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace ib {
namespace mw {
namespace test {

class MockComAdapter : public IComAdapter
{
public:
    void SendIbMessage(EndpointAddress from, sim::can::CanMessage&& msg) override
    {
        SendIbMessage_proxy(from, msg);
    }
    void SendIbMessage(EndpointAddress from, sim::eth::EthMessage&& msg) override
    {
        SendIbMessage_proxy(from, msg);
    }
    void SendIbMessage(EndpointAddress from, sim::fr::FrMessage&& msg) override
    {
        SendIbMessage_proxy(from, msg);
    }
    void SendIbMessage(EndpointAddress from, sim::fr::FrMessageAck&& msg) override
    {
        SendIbMessage_proxy(from, msg);
    }
    void SendIbMessage(EndpointAddress from, sim::io::PatternIoMessage&& msg) override
    {
        SendIbMessage_proxy(from, msg);
    }
    void SendIbMessage(EndpointAddress from, sim::generic::GenericMessage&& msg) override
    {
        SendIbMessage_proxy(from, msg);
    }

    MOCK_METHOD1(CreateCanController, sim::can::ICanController*(const std::string& canonicalName));
    MOCK_METHOD1(CreateEthController, sim::eth::IEthController*(const std::string& canonicalName));
    MOCK_METHOD1(CreateFlexrayController, sim::fr::IFrController*(const std::string& canonicalName));
    MOCK_METHOD1(CreateLinController, sim::lin::ILinController*(const std::string& canonicalName));
    MOCK_METHOD1(CreateAnalogIn, sim::io::IAnalogInPort*(const std::string& canonicalName));
    MOCK_METHOD1(CreateDigitalIn, sim::io::IDigitalInPort*(const std::string& canonicalName));
    MOCK_METHOD1(CreatePwmIn, sim::io::IPwmInPort*(const std::string& canonicalName));
    MOCK_METHOD1(CreatePatternIn, sim::io::IPatternInPort*(const std::string& canonicalName));
    MOCK_METHOD1(CreateAnalogOut, sim::io::IAnalogOutPort*(const std::string& canonicalName));
    MOCK_METHOD1(CreateDigitalOut, sim::io::IDigitalOutPort*(const std::string& canonicalName));
    MOCK_METHOD1(CreatePwmOut, sim::io::IPwmOutPort*(const std::string& canonicalName));
    MOCK_METHOD1(CreatePatternOut, sim::io::IPatternOutPort*(const std::string& canonicalName));
    MOCK_METHOD1(CreateGenericPublisher, sim::generic::IGenericPublisher*(const std::string& canonicalName));
    MOCK_METHOD1(CreateGenericSubscriber, sim::generic::IGenericSubscriber*(const std::string& canonicalName));

    MOCK_METHOD0(CreateSyncMaster, sync::ISyncMaster*());
    MOCK_METHOD0(GetParticipantController, sync::IParticipantController*());
    MOCK_METHOD0(GetSystemMonitor, sync::ISystemMonitor*());
    MOCK_METHOD0(GetSystemController, sync::ISystemController*());

    MOCK_METHOD1(RegisterCanSimulator, void(sim::can::IIbToCanSimulator*));
    MOCK_METHOD1(RegisterEthSimulator, void(sim::eth::IIbToEthSimulator*));
    MOCK_METHOD1(RegisterFlexraySimulator, void(sim::fr::IIbToFrBusSimulator*));
    MOCK_METHOD1(RegisterLinSimulator, void(sim::lin::IIbToLinSimulator*));

    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::can::CanMessage&));
    MOCK_METHOD2(SendIbMessage_proxy, void(EndpointAddress, sim::can::CanMessage&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::can::CanTransmitAcknowledge&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::can::CanControllerStatus&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::can::CanConfigureBaudrate&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::can::CanSetControllerMode&));

    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::eth::EthMessage&));
    MOCK_METHOD2(SendIbMessage_proxy, void(EndpointAddress, sim::eth::EthMessage&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::eth::EthTransmitAcknowledge&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::eth::EthStatus&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::eth::EthSetMode&));

    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::fr::FrMessage&));
    MOCK_METHOD2(SendIbMessage_proxy, void(EndpointAddress, sim::fr::FrMessage&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::fr::FrMessageAck&));
    MOCK_METHOD2(SendIbMessage_proxy, void(EndpointAddress, sim::fr::FrMessageAck&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::fr::FrSymbol&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::fr::FrSymbolAck&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::fr::HostCommand&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::fr::ControllerConfig&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::fr::TxBufferUpdate&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::fr::ControllerStatus&));

    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::lin::LinMessage&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::lin::RxRequest&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::lin::TxAcknowledge&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::lin::WakeupRequest&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::lin::ControllerConfig&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::lin::SlaveConfiguration&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::lin::SlaveResponse&));

    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::io::AnalogIoMessage&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::io::DigitalIoMessage&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::io::PatternIoMessage&));
    MOCK_METHOD2(SendIbMessage_proxy, void(EndpointAddress, sim::io::PatternIoMessage&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::io::PwmIoMessage&));

    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sim::generic::GenericMessage&));
    MOCK_METHOD2(SendIbMessage_proxy, void(EndpointAddress, sim::generic::GenericMessage&));
    
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sync::Tick&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sync::TickDone&));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sync::QuantumRequest& msg));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sync::QuantumGrant& msg));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sync::ParticipantStatus& msg));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sync::ParticipantCommand& msg));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const sync::SystemCommand& msg));

    MOCK_METHOD0(WaitForMessageDelivery, void());
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace test
} // namespace mw
} // namespace ib
