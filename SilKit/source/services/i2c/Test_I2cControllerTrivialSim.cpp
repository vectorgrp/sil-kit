// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/services/i2c/string_utils.hpp"
#include "silkit/util/HandlerId.hpp"

#include "MockParticipant.hpp"
#include "MockTraceSink.hpp"

#include "I2cController.hpp"
#include "WireI2cMessages.hpp"

namespace {

using namespace std::chrono_literals;

using testing::Return;
using testing::A;
using testing::An;
using testing::_;
using testing::InSequence;
using testing::NiceMock;

using namespace SilKit::Core;
using namespace SilKit::Services::I2c;
using SilKit::Util::HandlerId;

using SilKit::Core::Tests::DummyParticipant;

class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const WireI2cFrameEvent&));
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const I2cAcknowledge&));
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const WireI2cControllerConfig&));
};

class I2cControllerCallbacks
{
public:
    MOCK_METHOD2(FrameHandler, void(II2cController*, I2cFrameEvent));
    MOCK_METHOD2(FrameTransmitHandler, void(II2cController*, I2cFrameTransmitEvent));
};

auto MakeMasterController(MockParticipant& mockParticipant, SilKit::Config::I2cController cfg = {})
    -> std::unique_ptr<I2cController>
{
    auto ctrl = std::make_unique<I2cController>(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    I2cControllerConfig masterCfg{};
    masterCfg.mode = I2cControllerMode::Master;
    ctrl->Init(masterCfg);
    return ctrl;
}

TEST(Test_I2cControllerTrivialSim, write_to_registered_slave_receives_transmitted_ack)
{
    MockParticipant mockParticipant;
    I2cControllerCallbacks masterCallbacks;

    ServiceDescriptor masterDesc{};
    masterDesc.SetParticipantNameAndComputeId("master");
    masterDesc.SetNetworkName("I2cNetwork");
    masterDesc.SetServiceId(1);

    SilKit::Config::I2cController cfg;
    auto master = MakeMasterController(mockParticipant, cfg);
    master->SetServiceDescriptor(masterDesc);

    master->AddFrameHandler(
        std::bind(&I2cControllerCallbacks::FrameHandler, &masterCallbacks, std::placeholders::_1,
                  std::placeholders::_2),
        (SilKit::Services::DirectionMask)SilKit::Services::TransmitDirection::TX);
    master->AddFrameTransmitHandler(
        std::bind(&I2cControllerCallbacks::FrameTransmitHandler, &masterCallbacks, std::placeholders::_1,
                  std::placeholders::_2));

    // Frame event is broadcast via SendMsg; ack is delivered locally (no SendMsg for I2cAcknowledge)
    EXPECT_CALL(mockParticipant, SendMsg(master.get(), A<const WireI2cFrameEvent&>())).Times(1);
    EXPECT_CALL(mockParticipant.mockTimeProvider, Now()).Times(testing::AnyNumber());

    // Register slave address 0x42 with the master's trivial sim
    WireI2cControllerConfig slaveConfig{};
    slaveConfig.mode = I2cControllerMode::Slave;
    slaveConfig.slaveAddress = 0x42;
    master->ReceiveMsg(master.get(), slaveConfig);

    // TX handler fires (self-deliver TX); transmit handler fires (ack delivered directly)
    EXPECT_CALL(masterCallbacks, FrameHandler(master.get(), _)).Times(1);
    EXPECT_CALL(masterCallbacks, FrameTransmitHandler(master.get(), _)).Times(1);

    I2cFrame writeFrame{};
    writeFrame.address = 0x42;
    writeFrame.direction = I2cTransferDirection::Write;
    std::vector<uint8_t> payload{0xAB, 0xCD};
    writeFrame.data = SilKit::Util::Span<const uint8_t>(payload.data(), payload.size());

    master->SendFrame(writeFrame, nullptr);
}

TEST(Test_I2cControllerTrivialSim, write_to_unregistered_address_receives_address_nak)
{
    MockParticipant mockParticipant;
    I2cControllerCallbacks masterCallbacks;

    ServiceDescriptor masterDesc{};
    masterDesc.SetParticipantNameAndComputeId("master");
    masterDesc.SetNetworkName("I2cNetwork");
    masterDesc.SetServiceId(1);

    SilKit::Config::I2cController cfg;
    auto master = MakeMasterController(mockParticipant, cfg);
    master->SetServiceDescriptor(masterDesc);

    master->AddFrameTransmitHandler(
        std::bind(&I2cControllerCallbacks::FrameTransmitHandler, &masterCallbacks, std::placeholders::_1,
                  std::placeholders::_2));

    // No slave registered — ack is delivered locally (AddressNak)
    EXPECT_CALL(mockParticipant, SendMsg(master.get(), A<const WireI2cFrameEvent&>())).Times(1);
    EXPECT_CALL(mockParticipant.mockTimeProvider, Now()).Times(testing::AnyNumber());

    EXPECT_CALL(masterCallbacks,
                FrameTransmitHandler(master.get(),
                                     testing::Field(&I2cFrameTransmitEvent::status, I2cTransmitStatus::AddressNak)))
        .Times(1);

    I2cFrame writeFrame{};
    writeFrame.address = 0x77;
    writeFrame.direction = I2cTransferDirection::Write;

    master->SendFrame(writeFrame, nullptr);
}

TEST(Test_I2cControllerTrivialSim, init_broadcasts_controller_config)
{
    MockParticipant mockParticipant;

    ServiceDescriptor slaveDesc{};
    slaveDesc.SetParticipantNameAndComputeId("slave");
    slaveDesc.SetNetworkName("I2cNetwork");
    slaveDesc.SetServiceId(2);

    SilKit::Config::I2cController cfg;
    auto slave = std::make_unique<I2cController>(&mockParticipant, cfg, mockParticipant.GetTimeProvider());
    slave->SetServiceDescriptor(slaveDesc);

    WireI2cControllerConfig expectedConfig{};
    expectedConfig.mode = I2cControllerMode::Slave;
    expectedConfig.slaveAddress = 0x10;
    expectedConfig.slaveAddressMode = I2cAddressMode::AddressMode7Bit;
    expectedConfig.speedMode = I2cSpeedMode::Standard;

    EXPECT_CALL(mockParticipant, SendMsg(slave.get(), expectedConfig)).Times(1);
    EXPECT_CALL(mockParticipant.mockTimeProvider, Now()).Times(testing::AnyNumber());

    I2cControllerConfig slaveCfg{};
    slaveCfg.mode = I2cControllerMode::Slave;
    slaveCfg.slaveAddress = 0x10;
    slaveCfg.slaveAddressMode = I2cAddressMode::AddressMode7Bit;
    slaveCfg.speedMode = I2cSpeedMode::Standard;
    slave->Init(slaveCfg);
}

} // anonymous namespace
