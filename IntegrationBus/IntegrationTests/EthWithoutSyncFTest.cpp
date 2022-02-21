// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"

#include "EthDatatypeUtils.hpp"
#include "EthControllerFacade.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

#if IB_MW_HAVE_VASIO
#   include "VAsioRegistry.hpp"
#endif

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
        _testMessages.resize(10);
        for (auto index = 0u; index < _testMessages.size(); index++)
        {
            std::stringstream messageBuilder;
            messageBuilder << "Test Message " << index;
            std::string messageString = messageBuilder.str();
            auto& ethmsg = _testMessages[index].expectedMsg;
            auto sourceMac = ib::sim::eth::EthMac{ 0x9a, 0x89, 0x67, 0x45, 0x23, 0x12 };
            ethmsg.ethFrame.SetDestinationMac(ib::sim::eth::EthMac{ 0x12,0x23,0x45,0x67,0x89,0x9a });
            ethmsg.ethFrame.SetSourceMac(sourceMac);
            ethmsg.ethFrame.SetEtherType(0x8100);
            ethmsg.ethFrame.SetPayload(reinterpret_cast<const uint8_t*>(messageString.c_str()), messageString.size());
            ethmsg.timestamp = 1s;
            ethmsg.transmitId = index + 1;

            auto& ethack = _testMessages[index].expectedAck;
            ethack.sourceMac = sourceMac;
            ethack.timestamp = 1s;
            ethack.transmitId = index + 1;
        }
    }

    void EthWriter()
    {
        unsigned numSent{ 0 }, numAcks{ 0 };
        std::promise<void> ethWriterAllAcksReceivedPromiseLocal;
        
        auto comAdapter =
            ib::CreateSimulationParticipant(ib::cfg::CreateDummyConfiguration(), "EthWriter", _domainId, false);
        auto* controller = dynamic_cast<ib::sim::eth::EthControllerFacade*>(comAdapter->CreateEthController("ETH1"));

        controller->RegisterMessageAckHandler(
            [this, &ethWriterAllAcksReceivedPromiseLocal, &numAcks](ib::sim::eth::IEthController* /*ctrl*/, const ib::sim::eth::EthTransmitAcknowledge& ack) {
                _testMessages.at(numAcks++).receivedAck = ack;
                if (numAcks >= _testMessages.size())
                {
                    std::cout << "All eth acks received" << std::endl;
                    _ethWriterAllAcksReceivedPromise.set_value(); // Promise for ethReader
                    ethWriterAllAcksReceivedPromiseLocal.set_value(); // Promise for this thread
                }
            });

        _ethReaderRegisteredPromise.get_future().wait_for(10s);

        while (numSent < _testMessages.size())
        {
            controller->SendMessage(_testMessages.at(numSent++).expectedMsg); // Don't move the msg to test the altered transmitID
        }
        std::cout << "All eth messages sent" << std::endl;

        ethWriterAllAcksReceivedPromiseLocal.get_future().wait_for(10s);
        _ethReaderAllReceivedPromise.get_future().wait_for(10s);
    }

    void EthReader()
    {
        unsigned numReceived{ 0 };
        std::promise<void> ethReaderAllReceivedPromiseLocal;
        auto comAdapter =
            ib::CreateSimulationParticipant(ib::cfg::CreateDummyConfiguration(), "EthReader", _domainId, false);
        auto* controller = comAdapter->CreateEthController("ETH1");

        controller->RegisterReceiveMessageHandler(
            [this, &ethReaderAllReceivedPromiseLocal, &numReceived](ib::sim::eth::IEthController*, const ib::sim::eth::EthMessage& msg) {

                _testMessages.at(numReceived++).receivedMsg = msg;
                if (numReceived >= _testMessages.size())
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
        for (auto&& message : _testMessages)
        {
            EXPECT_EQ(message.expectedMsg, message.receivedMsg);
            EXPECT_EQ(message.expectedAck, message.receivedAck);
        }
    }

    struct Testmessage
    {
        ib::sim::eth::EthMessage expectedMsg;
        ib::sim::eth::EthMessage receivedMsg;
        ib::sim::eth::EthTransmitAcknowledge expectedAck;
        ib::sim::eth::EthTransmitAcknowledge receivedAck;
    };

    uint32_t _domainId;
    std::vector<Testmessage> _testMessages;
    std::promise<void> _ethReaderRegisteredPromise;
    std::promise<void> _ethReaderAllReceivedPromise;
    std::promise<void> _ethWriterAllAcksReceivedPromise;
};

TEST_F(EthWithoutSyncFTest, eth_communication_no_simulation_flow_vasio)
{
    auto registry = std::make_unique<ib::mw::VAsioRegistry>(ib::cfg::vasio::v1::CreateDummyIMiddlewareConfiguration());
    registry->ProvideDomain(_domainId);
    ExecuteTest();
}

} // anonymous namespace
