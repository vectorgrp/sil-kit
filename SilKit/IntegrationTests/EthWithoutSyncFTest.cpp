// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"

#include "EthDatatypeUtils.hpp"
#include "EthController.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

#include "VAsioRegistry.hpp"

#include "ConfigurationTestUtils.hpp"

namespace {

using namespace std::chrono_literals;

class EthWithoutSyncFTest : public testing::Test
{
protected:

    EthWithoutSyncFTest()
    {
        _registryUri = MakeTestRegistryUri();
        SetupTestData();
    }

    void SetupTestData()
    {
        _testFrames.resize(10);
        for (auto index = 0u; index < _testFrames.size(); index++)
        {
            std::stringstream messageBuilder;
            messageBuilder << "Test Message " << index;
            std::string messageString = messageBuilder.str();
            auto& frameEvent = _testFrames[index].expectedFrameEvent;

            SilKit::Services::Ethernet::EthernetMac destinationMac{ 0x12, 0x23, 0x45, 0x67, 0x89, 0x9a };
            SilKit::Services::Ethernet::EthernetMac sourceMac{ 0x9a, 0x89, 0x67, 0x45, 0x23, 0x12 };
            SilKit::Services::Ethernet::EthernetEtherType etherType{ 0x0800 };
            SilKit::Services::Ethernet::EthernetVlanTagControlIdentifier tci{ 0x0000 };

            frameEvent.frame = SilKit::Services::Ethernet::CreateEthernetFrameWithVlanTag(destinationMac, sourceMac, etherType, messageString, tci);
            frameEvent.transmitId = index + 1;

            auto& ethack = _testFrames[index].expectedAck;
            ethack.sourceMac = sourceMac;
            ethack.transmitId = index + 1;
        }
    }

    void EthWriter()
    {
        unsigned numSent{ 0 }, numAcks{ 0 };
        std::promise<void> ethWriterAllAcksReceivedPromiseLocal;
        
        auto participant =
            SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(), "EthWriter", _registryUri);
        auto* controller = dynamic_cast<SilKit::Services::Ethernet::EthController*>(
            participant->CreateEthernetController("ETH1", "ETH1"));

        controller->Activate();

        controller->AddFrameTransmitHandler(
            [this, &ethWriterAllAcksReceivedPromiseLocal, &numAcks](SilKit::Services::Ethernet::IEthernetController* /*ctrl*/, const SilKit::Services::Ethernet::EthernetFrameTransmitEvent& ack) {
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
            controller->SendFrameEvent(_testFrames.at(numSent++).expectedFrameEvent); // Don't move the event to test the altered transmitID
        }
        std::cout << "All eth messages sent" << std::endl;

        ethWriterAllAcksReceivedPromiseLocal.get_future().wait_for(10s);
        _ethReaderAllReceivedPromise.get_future().wait_for(10s);
    }

    void EthReader()
    {
        unsigned numReceived{ 0 };
        std::promise<void> ethReaderAllReceivedPromiseLocal;
        auto participant =
            SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(), "EthReader", _registryUri);
        auto* controller = participant->CreateEthernetController("ETH1", "ETH1");

        controller->Activate();

        controller->AddFrameHandler(
            [this, &ethReaderAllReceivedPromiseLocal, &numReceived](SilKit::Services::Ethernet::IEthernetController*, const SilKit::Services::Ethernet::EthernetFrameEvent& msg) {

                _testFrames.at(numReceived++).receivedFrameEvent = msg;
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
        std::thread ethReaderThread{ [this] { EthReader(); } };
        std::thread ethWriterThread{ [this] { EthWriter(); } };
        ethReaderThread.join();
        ethWriterThread.join();
        for (auto&& message : _testFrames)
        {
            // Without sync: Do not test the timestamps
            message.receivedFrameEvent.timestamp = 0ns;
            message.receivedAck.timestamp = 0ns;
            EXPECT_EQ(message.expectedFrameEvent, message.receivedFrameEvent);
            EXPECT_EQ(message.expectedAck, message.receivedAck);
        }
    }

    struct TestFrame
    {
        SilKit::Services::Ethernet::EthernetFrameEvent expectedFrameEvent;
        SilKit::Services::Ethernet::EthernetFrameEvent receivedFrameEvent;
        SilKit::Services::Ethernet::EthernetFrameTransmitEvent expectedAck;
        SilKit::Services::Ethernet::EthernetFrameTransmitEvent receivedAck;
    };

    std::string  _registryUri;
    std::vector<TestFrame> _testFrames;
    std::promise<void> _ethReaderRegisteredPromise;
    std::promise<void> _ethReaderAllReceivedPromise;
    std::promise<void> _ethWriterAllAcksReceivedPromise;
};

TEST_F(EthWithoutSyncFTest, eth_communication_no_simulation_flow_vasio)
{
    auto registry = std::make_unique<SilKit::Core::VAsioRegistry>(SilKit::Config::MakeEmptyParticipantConfiguration());
    registry->ProvideDomain(_registryUri);
    ExecuteTest();
}

} // anonymous namespace