// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"

#include "CanDatatypesUtils.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"
#include "MockParticipantConfiguration.hpp"

#if IB_MW_HAVE_VASIO
#   include "VAsioRegistry.hpp"
#endif

namespace {

using namespace std::chrono_literals;

// basically the same as the normal == operator, but it doesn't compare timestamps
bool Matches(const ib::sim::can::CanMessage& lhs, const ib::sim::can::CanMessage& rhs)
{
    return lhs.transmitId == rhs.transmitId 
        && lhs.canId == rhs.canId
        && lhs.flags == rhs.flags 
        && lhs.dlc == rhs.dlc 
        && lhs.dataField == rhs.dataField
        && lhs.userContext == rhs.userContext;
}

class CanWithoutSyncFTest : public testing::Test
{
protected:

    CanWithoutSyncFTest()
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
            auto& canmsg = _testMessages[index].expectedMsg;
            canmsg.canId = index;
            canmsg.dataField.assign(messageString.begin(), messageString.end());
            canmsg.dlc = canmsg.dataField.size();
            canmsg.flags = ib::sim::can::CanMessage::CanReceiveFlags{ 1,0,1,0,1 };
            canmsg.timestamp = 1s;
            canmsg.transmitId = index + 1;
            canmsg.direction = ib::sim::TransmitDirection::RX;
            canmsg.userContext = (void*)((size_t)index+1);

            auto& canack = _testMessages[index].expectedAck;
            canack.canId = index;
            canack.timestamp = 1s;
            canack.transmitId = index + 1;
            canack.status = ib::sim::can::CanTransmitStatus::Transmitted;
            canack.userContext = (void*)((size_t)index+1);
        }
    }

    void CanWriter()
    {
        unsigned numSent{ 0 }, numAcks{ 0 };
        std::promise<void> canWriterAllAcksReceivedPromiseLocal;

        auto participant =
            ib::CreateParticipant(ib::cfg::MockParticipantConfiguration(), "CanWriter", _domainId, false);
        auto* controller = participant->CreateCanController("CAN1");

        controller->RegisterTransmitStatusHandler(
            [this, &canWriterAllAcksReceivedPromiseLocal, &numAcks](ib::sim::can::ICanController* /*ctrl*/, const ib::sim::can::CanTransmitAcknowledge& ack) {
                _testMessages.at(numAcks++).receivedAck = ack;
                if (numAcks >= _testMessages.size())
                {
                    std::cout << "All can acks received" << std::endl;
                    _canWriterAllAcksReceivedPromise.set_value(); // Promise for canReader
                    canWriterAllAcksReceivedPromiseLocal.set_value();
                }
            });

        _canReaderRegisteredPromise.get_future().wait_for(1min);

        while (numSent < _testMessages.size())
        {
            controller->SendMessage(_testMessages.at(numSent).expectedMsg, (void*)((size_t)numSent+1)); // Don't move the msg to test the altered transmitID
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

        auto participant = ib::CreateParticipant(ib::cfg::MockParticipantConfiguration(), "CanReader", _domainId, false);
        auto* controller = participant->CreateCanController("CAN1", "CAN1");

        controller->RegisterReceiveMessageHandler(
            [this, &canReaderAllReceivedPromiseLocal, &numReceived](ib::sim::can::ICanController*, const ib::sim::can::CanMessage& msg) {

                _testMessages.at(numReceived++).receivedMsg = msg;
                if (numReceived >= _testMessages.size())
                {
                    std::cout << "All can messages received" << std::endl;
                    _canReaderAllReceivedPromise.set_value();
                    canReaderAllReceivedPromiseLocal.set_value();
                }
            });

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
        ib::sim::can::CanMessage expectedMsg;
        ib::sim::can::CanMessage receivedMsg;
        ib::sim::can::CanTransmitAcknowledge expectedAck;
        ib::sim::can::CanTransmitAcknowledge receivedAck;
    };

    uint32_t _domainId;
    std::vector<Testmessage> _testMessages;
    std::promise<void> _canReaderRegisteredPromise;
    std::promise<void> _canReaderAllReceivedPromise;
    std::promise<void> _canWriterAllAcksReceivedPromise;
};

TEST_F(CanWithoutSyncFTest, can_communication_no_simulation_flow_vasio)
{
    auto registry = std::make_unique<ib::mw::VAsioRegistry>(ib::cfg::ParticipantConfigurationFromString("ParticipantName: Registry"));
    registry->ProvideDomain(_domainId);
    ExecuteTest();
}

} // anonymous namespace
