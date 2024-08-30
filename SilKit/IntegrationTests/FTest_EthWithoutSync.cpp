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

#include "EthernetHelpers.hpp"

namespace {

using namespace std::chrono_literals;

using SilKit::Util::ItemsAreEqual;

using SilKit::IntegrationTests::EthernetMac;
using SilKit::IntegrationTests::EthernetEtherType;
using SilKit::IntegrationTests::EthernetVlanTagControlIdentifier;
using SilKit::IntegrationTests::CreateEthernetFrameWithVlanTagFromString;

constexpr std::size_t MINIMUM_ETHERNET_FRAME_LENGTH = 60;

class FTest_EthWithoutSync : public testing::Test
{
protected:
    FTest_EthWithoutSync()
    {
        SetupTestData();
    }

    void SetupTestData()
    {
        _testFrames.resize(10);
        for (size_t index = 0u; index < _testFrames.size(); index++)
        {
            std::stringstream messageBuilder;
            messageBuilder << "Test Message " << index;
            std::string messageString = messageBuilder.str();
            // pad the message such that the actual frame has exactly the minimum frame length
            messageString.resize(std::max<size_t>(messageString.size(), MINIMUM_ETHERNET_FRAME_LENGTH - 18), ' ');

            auto& frameEvent = _testFrames[index].expectedFrameEvent;
            auto& frameData = _testFrames[index].expectedFrameData;

            EthernetMac destinationMac{0x12, 0x23, 0x45, 0x67, 0x89, 0x9a};
            EthernetMac sourceMac{0x9a, 0x89, 0x67, 0x45, 0x23, 0x12};
            EthernetEtherType etherType{0x0800};
            EthernetVlanTagControlIdentifier tci{0x0000};

            frameData =
                CreateEthernetFrameWithVlanTagFromString(destinationMac, sourceMac, etherType, messageString, tci);
            frameEvent.frame = SilKit::Services::Ethernet::EthernetFrame{frameData};

            EXPECT_GE(frameEvent.frame.raw.size(), MINIMUM_ETHERNET_FRAME_LENGTH);
            frameEvent.userContext = reinterpret_cast<void*>(static_cast<uintptr_t>(index + 1));

            auto& ethack = _testFrames[index].expectedAck;
            ethack.userContext = reinterpret_cast<void*>(static_cast<uintptr_t>(index + 1));
            ethack.status = SilKit::Services::Ethernet::EthernetTransmitStatus::Transmitted;
        }
    }

    void EthWriter()
    {
        unsigned numSent{0}, numAcks{0};
        std::promise<void> ethWriterAllAcksReceivedPromiseLocal;

        const auto participant = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""),
                                                           "EthWriter", _registryUri);
        const auto controller = participant->CreateEthernetController("ETH1", "ETH1");

        controller->Activate();

        controller->AddFrameTransmitHandler([this, &ethWriterAllAcksReceivedPromiseLocal, &numAcks](
                                                SilKit::Services::Ethernet::IEthernetController* /*ctrl*/,
                                                const SilKit::Services::Ethernet::EthernetFrameTransmitEvent& ack) {
            _testFrames.at(numAcks++).receivedAck = ack;
            if (numAcks >= _testFrames.size())
            {
                std::cout << "All eth acks received" << std::endl;
                _ethWriterAllAcksReceivedPromise.set_value(); // Promise for ethReader
                ethWriterAllAcksReceivedPromiseLocal.set_value(); // Promise for this thread
            }
        });

        _ethReaderRegisteredPromise.get_future().wait_for(10s);

        while (numSent < _testFrames.size())
        {
            const auto& frameEvent = _testFrames.at(numSent).expectedFrameEvent;
            controller->SendFrame(frameEvent.frame, reinterpret_cast<void*>(frameEvent.userContext));
            ++numSent;
        }
        std::cout << "All eth messages sent" << std::endl;

        ethWriterAllAcksReceivedPromiseLocal.get_future().wait_for(10s);
        _ethReaderAllReceivedPromise.get_future().wait_for(10s);
    }

    void EthReader()
    {
        unsigned numReceived{0};
        std::promise<void> ethReaderAllReceivedPromiseLocal;
        const auto participant = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""),
                                                           "EthReader", _registryUri);
        const auto controller = participant->CreateEthernetController("ETH1", "ETH1");

        controller->Activate();

        controller->AddFrameHandler([this, &ethReaderAllReceivedPromiseLocal, &numReceived](
                                        SilKit::Services::Ethernet::IEthernetController*,
                                        const SilKit::Services::Ethernet::EthernetFrameEvent& msg) {
            unsigned frameIndex = numReceived++;

            auto& frameData = _testFrames.at(frameIndex).receivedFrameData;
            auto& frameEvent = _testFrames.at(frameIndex).receivedFrameEvent;

            frameEvent = msg;
            frameData = ToStdVector(msg.frame.raw);
            frameEvent.frame = SilKit::Services::Ethernet::EthernetFrame{frameData};

            if (numReceived >= _testFrames.size())
            {
                std::cout << "All eth messages received" << std::endl;
                _ethReaderAllReceivedPromise.set_value(); // Promise for ethWriter
                ethReaderAllReceivedPromiseLocal.set_value(); // Promise for this thread
            }
        });

        _ethReaderRegisteredPromise.set_value();

        _ethWriterAllAcksReceivedPromise.get_future().wait_for(10s);

        ethReaderAllReceivedPromiseLocal.get_future().wait_for(10s);
    }

    void ExecuteTest()
    {
        std::thread ethReaderThread{[this] { EthReader(); }};
        std::thread ethWriterThread{[this] { EthWriter(); }};
        ethReaderThread.join();
        ethWriterThread.join();
        for (auto&& message : _testFrames)
        {
            // Without sync: Do not test the timestamps
            message.receivedFrameEvent.timestamp = 0ns;
            message.receivedAck.timestamp = 0ns;

            EXPECT_TRUE(ItemsAreEqual(message.expectedFrameEvent.frame.raw, message.receivedFrameEvent.frame.raw));
            EXPECT_EQ(message.expectedFrameEvent.timestamp, message.receivedFrameEvent.timestamp);

            EXPECT_EQ(message.expectedAck.timestamp, message.receivedAck.timestamp);
            EXPECT_EQ(message.expectedAck.status, message.receivedAck.status);
        }
    }

    struct TestFrame
    {
        std::vector<uint8_t> expectedFrameData;
        std::vector<uint8_t> receivedFrameData;
        SilKit::Services::Ethernet::EthernetFrameEvent expectedFrameEvent;
        SilKit::Services::Ethernet::EthernetFrameEvent receivedFrameEvent;
        SilKit::Services::Ethernet::EthernetFrameTransmitEvent expectedAck;
        SilKit::Services::Ethernet::EthernetFrameTransmitEvent receivedAck;
    };

    std::string _registryUri;
    std::vector<TestFrame> _testFrames;
    std::promise<void> _ethReaderRegisteredPromise;
    std::promise<void> _ethReaderAllReceivedPromise;
    std::promise<void> _ethWriterAllAcksReceivedPromise;
};

TEST_F(FTest_EthWithoutSync, eth_communication_no_simulation_flow_vasio)
{
    auto registry =
        SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
    _registryUri = registry->StartListening("silkit://localhost:0");
    ExecuteTest();
}

} // anonymous namespace
