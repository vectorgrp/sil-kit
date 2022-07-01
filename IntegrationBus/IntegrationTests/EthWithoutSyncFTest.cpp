// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"

#include "EthDatatypeUtils.hpp"
#include "EthController.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

#include "VAsioRegistry.hpp"

#include "MockParticipantConfiguration.hpp"

namespace {

using namespace std::chrono_literals;

class EthWithoutSyncFTest : public testing::Test
{
protected:

    EthWithoutSyncFTest()
    {
        _domainId = static_cast<uint32_t>(GetTestPid());
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

            ib::sim::eth::EthernetMac destinationMac{ 0x12, 0x23, 0x45, 0x67, 0x89, 0x9a };
            ib::sim::eth::EthernetMac sourceMac{ 0x9a, 0x89, 0x67, 0x45, 0x23, 0x12 };
            ib::sim::eth::EthernetEtherType etherType{ 0x0800 };
            ib::sim::eth::EthernetVlanTagControlIdentifier tci{ 0x0000 };

            frameEvent.frame = ib::sim::eth::CreateEthernetFrameWithVlanTag(destinationMac, sourceMac, etherType, messageString, tci);
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
            ib::CreateParticipant(ib::cfg::MockParticipantConfiguration(), "EthWriter", _domainId);
        auto* controller = dynamic_cast<ib::sim::eth::EthController*>(participant->CreateEthernetController("ETH1"));

        controller->AddFrameTransmitHandler(
            [this, &ethWriterAllAcksReceivedPromiseLocal, &numAcks](ib::sim::eth::IEthernetController* /*ctrl*/, const ib::sim::eth::EthernetFrameTransmitEvent& ack) {
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
            ib::CreateParticipant(ib::cfg::MockParticipantConfiguration(), "EthReader", _domainId);
        auto* controller = participant->CreateEthernetController("ETH1");

        controller->AddFrameHandler(
            [this, &ethReaderAllReceivedPromiseLocal, &numReceived](ib::sim::eth::IEthernetController*, const ib::sim::eth::EthernetFrameEvent& msg) {

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
        ib::sim::eth::EthernetFrameEvent expectedFrameEvent;
        ib::sim::eth::EthernetFrameEvent receivedFrameEvent;
        ib::sim::eth::EthernetFrameTransmitEvent expectedAck;
        ib::sim::eth::EthernetFrameTransmitEvent receivedAck;
    };

    uint32_t _domainId;
    std::vector<TestFrame> _testFrames;
    std::promise<void> _ethReaderRegisteredPromise;
    std::promise<void> _ethReaderAllReceivedPromise;
    std::promise<void> _ethWriterAllAcksReceivedPromise;
};

TEST_F(EthWithoutSyncFTest, eth_communication_no_simulation_flow_vasio)
{
    auto registry = std::make_unique<ib::mw::VAsioRegistry>(ib::cfg::MockParticipantConfiguration());
    registry->ProvideDomain(_domainId);
    ExecuteTest();
}

} // anonymous namespace
