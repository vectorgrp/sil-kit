// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"

#include "CanDatatypesUtils.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"
#include "ConfigurationTestUtils.hpp"

#include "VAsioRegistry.hpp"

namespace {

using namespace std::chrono_literals;

// basically the same as the normal == operator, but it doesn't compare timestamps
bool Matches(const SilKit::Services::Can::WireCanFrameEvent& lhs, const SilKit::Services::Can::WireCanFrameEvent& rhs)
{
    return lhs.frame.canId == rhs.frame.canId
        && lhs.frame.flags == rhs.frame.flags
        && lhs.frame.dlc == rhs.frame.dlc
        && SilKit::Util::ItemsAreEqual(lhs.frame.dataField, rhs.frame.dataField)
        && lhs.userContext == rhs.userContext 
        && lhs.direction == rhs.direction;
}

class CanWithoutSyncFTest : public testing::Test
{
protected:

    CanWithoutSyncFTest()
    {
        _registryUri = MakeTestRegistryUri();
        SetupTestData();
    }


    void SetupTestData()
    {
        _testMessages.resize(10);
        for (auto index = 0u; index < _testMessages.size(); index++)
        {
            std::stringstream messageBuilder;
            messageBuilder << "Test Message " << index;
            std::string messageString = messageBuilder.str();

            std::vector<uint8_t> messageBytes;
            messageBytes.resize(messageString.size());
            std::copy(messageString.begin(), messageString.end(), messageBytes.begin());

            using SilKit::Services::Can::CanFrameFlag;
            using SilKit::Services::Can::CanFrameFlagMask;

            auto& canmsg = _testMessages[index].expectedMsg;
            canmsg.frame.canId = index;
            canmsg.frame.dataField = messageBytes;
            canmsg.frame.dlc = static_cast<uint16_t>(canmsg.frame.dataField.AsSpan().size());
            canmsg.frame.flags |=
                static_cast<CanFrameFlagMask>(CanFrameFlag::Ide) | static_cast<CanFrameFlagMask>(CanFrameFlag::Fdf)
                | static_cast<CanFrameFlagMask>(CanFrameFlag::Esi) | static_cast<CanFrameFlagMask>(CanFrameFlag::Sec);
            canmsg.timestamp = 1s;
            canmsg.direction = SilKit::Services::TransmitDirection::RX;
            canmsg.userContext = (void*)((size_t)index+1);

            auto& canack = _testMessages[index].expectedAck;
            canack.canId = index;
            canack.timestamp = 1s;
            canack.status = SilKit::Services::Can::CanTransmitStatus::Transmitted;
            canack.userContext = (void*)((size_t)index+1);
        }
    }

    void CanWriter()
    {
        unsigned numSent{ 0 }, numAcks{ 0 };
        std::promise<void> canWriterAllAcksReceivedPromiseLocal;

        auto participant =
            SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(), "CanWriter", _registryUri);
        auto* controller = participant->CreateCanController("CAN1", "CAN1");

        controller->AddFrameTransmitHandler(
            [this, &canWriterAllAcksReceivedPromiseLocal, &numAcks](SilKit::Services::Can::ICanController* /*ctrl*/, const SilKit::Services::Can::CanFrameTransmitEvent& ack) {
                _testMessages.at(numAcks++).receivedAck = ack;
                if (numAcks >= _testMessages.size())
                {
                    std::cout << "All can acks received" << std::endl;
                    _canWriterAllAcksReceivedPromise.set_value(); // Promise for canReader
                    canWriterAllAcksReceivedPromiseLocal.set_value();
                }
            });

        controller->Start();

        _canReaderRegisteredPromise.get_future().wait_for(1min);

        while (numSent < _testMessages.size())
        {
            controller->SendFrame(ToCanFrame(_testMessages.at(numSent).expectedMsg.frame), (void*)((size_t)numSent+1)); // Don't move the msg to test the altered transmitID
            numSent++;
        }
        std::cout << "All can messages sent" << std::endl;

        canWriterAllAcksReceivedPromiseLocal.get_future().wait_for(10s);
        _canReaderAllReceivedPromise.get_future().wait_for(10s);
    }

    void CanReader()
    {
        std::promise<void> canReaderAllReceivedPromiseLocal;
        unsigned numReceived{ 0 };

        auto participant = SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(), "CanReader", _registryUri);
        auto* controller = participant->CreateCanController("CAN1", "CAN1");

        controller->AddFrameHandler(
            [this, &canReaderAllReceivedPromiseLocal, &numReceived](SilKit::Services::Can::ICanController*, const SilKit::Services::Can::CanFrameEvent& msg) {

                _testMessages.at(numReceived++).receivedMsg = MakeWireCanFrameEvent(msg);
                if (numReceived >= _testMessages.size())
                {
                    std::cout << "All can messages received" << std::endl;
                    _canReaderAllReceivedPromise.set_value();
                    canReaderAllReceivedPromiseLocal.set_value();
                }
        });

        controller->Start();

        _canReaderRegisteredPromise.set_value();

        _canWriterAllAcksReceivedPromise.get_future().wait_for(10s);

        canReaderAllReceivedPromiseLocal.get_future().wait_for(10s);
    }

    void ExecuteTest()
    {
        std::thread canReaderThread{ [this] { CanReader(); } };
        std::thread canWriterThread{ [this] { CanWriter(); } };
        canReaderThread.join();
        canWriterThread.join();
        for (auto&& message : _testMessages)
        {
            EXPECT_TRUE(Matches(message.expectedMsg, message.receivedMsg));
            EXPECT_EQ(message.expectedAck, message.receivedAck);
        }
    }

    struct Testmessage
    {
        SilKit::Services::Can::WireCanFrameEvent expectedMsg;
        SilKit::Services::Can::WireCanFrameEvent receivedMsg;
        SilKit::Services::Can::CanFrameTransmitEvent expectedAck;
        SilKit::Services::Can::CanFrameTransmitEvent receivedAck;
    };

    std::string _registryUri;
    std::vector<Testmessage> _testMessages;
    std::promise<void> _canReaderRegisteredPromise;
    std::promise<void> _canReaderAllReceivedPromise;
    std::promise<void> _canWriterAllAcksReceivedPromise;
};

TEST_F(CanWithoutSyncFTest, can_communication_no_simulation_flow_vasio)
{
    auto registry = std::make_unique<SilKit::Core::VAsioRegistry>(SilKit::Config::ParticipantConfigurationFromString("ParticipantName: Registry"));
    registry->StartListening(_registryUri);
    ExecuteTest();
}

} // anonymous namespace
