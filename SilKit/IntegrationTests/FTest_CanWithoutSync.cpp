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

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;

// basically the same as the normal == operator, but it doesn't compare timestamps
bool Matches(const SilKit::Services::Can::CanFrameEvent& lhs, const SilKit::Services::Can::CanFrameEvent& rhs)
{
    return lhs.frame.canId == rhs.frame.canId && lhs.frame.flags == rhs.frame.flags && lhs.frame.dlc == rhs.frame.dlc
           && SilKit::Util::ItemsAreEqual(lhs.frame.dataField, rhs.frame.dataField)
           && lhs.userContext == rhs.userContext && lhs.direction == rhs.direction;
}

class FTest_CanWithoutSync : public testing::Test
{
protected:
    FTest_CanWithoutSync()
    {
        SetupTestData();
    }


    void SetupTestData()
    {
        _testMessages.resize(10);
        for (auto index = 0u; index < _testMessages.size(); index++)
        {
            auto& canmsgData = _testMessages[index].expectedMsgData;

            std::stringstream messageBuilder;
            messageBuilder << "Test Message " << index;
            std::string messageString = messageBuilder.str();

            canmsgData.resize(messageString.size());
            std::copy(messageString.begin(), messageString.end(), canmsgData.begin());

            using SilKit::Services::Can::CanFrameFlag;
            using SilKit::Services::Can::CanFrameFlagMask;

            auto& canmsg = _testMessages[index].expectedMsg;

            canmsg.frame.canId = index;
            canmsg.frame.dataField = canmsgData;
            canmsg.frame.dlc = static_cast<uint16_t>(canmsg.frame.dataField.size());
            canmsg.frame.flags |=
                static_cast<CanFrameFlagMask>(CanFrameFlag::Ide) | static_cast<CanFrameFlagMask>(CanFrameFlag::Fdf)
                | static_cast<CanFrameFlagMask>(CanFrameFlag::Esi) | static_cast<CanFrameFlagMask>(CanFrameFlag::Sec);
            canmsg.timestamp = 1s;
            canmsg.direction = SilKit::Services::TransmitDirection::RX;
            canmsg.userContext = 0;

            auto& canack = _testMessages[index].expectedAck;
            canack.canId = index;
            canack.timestamp = 1s;
            canack.status = SilKit::Services::Can::CanTransmitStatus::Transmitted;
            canack.userContext = (void*)((size_t)index + 1);
        }
    }

    void CanWriter()
    {
        unsigned numSent{0}, numAcks{0};
        std::promise<void> canWriterAllAcksReceivedPromiseLocal;

        auto participant = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""),
                                                     "CanWriter", _registryUri);
        auto* controller = participant->CreateCanController("CAN1", "CAN1");

        controller->AddFrameTransmitHandler([this, &canWriterAllAcksReceivedPromiseLocal, &numAcks](
                                                SilKit::Services::Can::ICanController* /*ctrl*/,
                                                const SilKit::Services::Can::CanFrameTransmitEvent& ack) {
            _testMessages.at(numAcks++).receivedAck = ack;
            auto tempUserContext = ack.userContext;
            // double check that userContext is not nullified for frame transmit handler
            EXPECT_TRUE(tempUserContext == (void*)((size_t)numAcks));
            if (numAcks >= _testMessages.size())
            {
                std::cout << "All can acks received" << std::endl;
                _canWriterAllAcksReceivedPromise.set_value(); // Promise for canReader
                canWriterAllAcksReceivedPromiseLocal.set_value();
            }
        });


        controller->AddFrameHandler(
            [](SilKit::Services::Can::ICanController*, const SilKit::Services::Can::CanFrameEvent& msg) {
            // make sure that userContext fo TX is not nullified
            EXPECT_TRUE(msg.userContext > (void*)((size_t)0));
        }, static_cast<SilKit::Services::DirectionMask>(SilKit::Services::TransmitDirection::TX));

        controller->Start();

        _canReaderRegisteredPromise.get_future().wait_for(1min);

        while (numSent < _testMessages.size())
        {
            controller->SendFrame(_testMessages.at(numSent).expectedMsg.frame,
                                  (void*)((size_t)numSent + 1)); // Don't move the msg to test the altered transmitID
            numSent++;
        }
        std::cout << "All can messages sent" << std::endl;

        canWriterAllAcksReceivedPromiseLocal.get_future().wait_for(10s);
        _canReaderAllReceivedPromise.get_future().wait_for(10s);
    }

    void CanReader()
    {
        std::promise<void> canReaderAllReceivedPromiseLocal;
        unsigned numReceived{0};

        auto participant = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""),
                                                     "CanReader", _registryUri);
        auto* controller = participant->CreateCanController("CAN1", "CAN1");

        controller->AddFrameHandler(
            [this, &canReaderAllReceivedPromiseLocal, &numReceived](SilKit::Services::Can::ICanController*,
                                                                    const SilKit::Services::Can::CanFrameEvent& msg) {
            unsigned msgIndex = numReceived++;

            auto& msgData = _testMessages.at(msgIndex).receivedMsgData;
            auto& msgEvent = _testMessages.at(msgIndex).receivedMsg;

            msgEvent = msg;

            msgData = ToStdVector(msg.frame.dataField);
            msgEvent.frame.dataField = msgData;

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
        std::thread canReaderThread{[this] { CanReader(); }};
        std::thread canWriterThread{[this] { CanWriter(); }};
        canReaderThread.join();
        canWriterThread.join();
        for (auto&& message : _testMessages)
        {
            EXPECT_TRUE(Matches(message.expectedMsg, message.receivedMsg));

            EXPECT_EQ(message.expectedAck.status, message.receivedAck.status);
            EXPECT_EQ(message.expectedAck.userContext, message.receivedAck.userContext);
        }
    }

    struct Testmessage
    {
        std::vector<uint8_t> expectedMsgData;
        std::vector<uint8_t> receivedMsgData;
        SilKit::Services::Can::CanFrameEvent expectedMsg;
        SilKit::Services::Can::CanFrameEvent receivedMsg;
        SilKit::Services::Can::CanFrameTransmitEvent expectedAck;
        SilKit::Services::Can::CanFrameTransmitEvent receivedAck;
    };

    std::string _registryUri;
    std::vector<Testmessage> _testMessages;
    std::promise<void> _canReaderRegisteredPromise;
    std::promise<void> _canReaderAllReceivedPromise;
    std::promise<void> _canWriterAllAcksReceivedPromise;
};

TEST_F(FTest_CanWithoutSync, can_communication_no_simulation_flow_vasio)
{
    auto registry =
        SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
    _registryUri = registry->StartListening("silkit://localhost:0");
    ExecuteTest();
}

} // anonymous namespace
