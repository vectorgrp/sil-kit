// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"

#include "SimTestHarness.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

namespace {

using namespace std::chrono_literals;
using namespace ib::mw;

using testing::_;
using testing::A;
using testing::An;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

class IoMessageITest : public testing::Test
{
protected:
    IoMessageITest()
        : topics(4)
    {
        topics[0].name = "DIO1";
        topics[1].name = "DIO2";
        topics[2].name = "AIO1";
        topics[3].name = "AIO2";

        ib::cfg::ConfigBuilder cfgBuilder("IoMessageTestConfig");
        auto&& simulationSetup = cfgBuilder.SimulationSetup();
        simulationSetup.AddParticipant("Sender")
            ->AddDigitalOut(topics[0].name).WithInitValue(true)
            ->AddDigitalOut(topics[1].name).WithInitValue(false)
            ->AddAnalogOut(topics[2].name).WithInitValue(5.0)
            ->AddAnalogOut(topics[3].name).WithInitValue(17.3);
        simulationSetup.AddParticipant("Receiver")
            ->AddDigitalIn(topics[0].name)
            ->AddDigitalIn(topics[1].name)
            ->AddAnalogIn(topics[2].name)
            ->AddAnalogIn(topics[3].name);

        ibConfig = cfgBuilder.Build();
    }

    struct Topic
    {
        std::string name;
        unsigned numUpdates{0};
        bool testOK = false;
    };
    
    template<class InPortT>
    static void SetExpectation(Topic& topic, InPortT* port, const typename InPortT::ValueType& expectedValue)
    {
        port->RegisterHandler(
            [&topic, expectedValue](InPortT* /*port*/, const typename InPortT::ValueType& data)
            {
                topic.testOK = (data == expectedValue);
                topic.numUpdates++;
                EXPECT_EQ(data, expectedValue) 
                    << "Topic: " 
                    << topic.name
                    << " valueUpdates: " << topic.numUpdates ;
            }
        );
    }

    void RunTest(ib::cfg::Middleware middleware)
    {
        ibConfig.middlewareConfig.activeMiddleware = middleware;

        const uint32_t domainId = static_cast<uint32_t>(GetTestPid());
        ib::test::SimTestHarness testHarness(ibConfig, domainId);


        // set up publisher with initial values
        auto* pubComAdapter = testHarness.GetParticipant("Sender")->ComAdapter();
        //Creating a port will write the initial value to it
        (void)pubComAdapter->CreateDigitalOut(topics[0].name);
        (void)pubComAdapter->CreateDigitalOut(topics[1].name);
        (void)pubComAdapter->CreateAnalogOut(topics[2].name);
        (void)pubComAdapter->CreateAnalogOut(topics[3].name);

        //set up expectations of initial port values on subscriber side
        auto* receiver = testHarness.GetParticipant("Receiver");
        auto* subComAdapter = receiver->ComAdapter();

        auto* dio1 = subComAdapter->CreateDigitalIn(topics[0].name);
        auto* dio2 = subComAdapter->CreateDigitalIn(topics[1].name);
        auto* aio1 = subComAdapter->CreateAnalogIn(topics[2].name);
        auto* aio2 = subComAdapter->CreateAnalogIn(topics[3].name);

        SetExpectation(topics[0], dio1, true);
        SetExpectation(topics[1], dio2, false);
        SetExpectation(topics[2], aio1, 5.0);
        SetExpectation(topics[3], aio2, 17.3);

        subComAdapter->GetParticipantController()->SetSimulationTask(
            [receiver, this](auto now, auto)
            {
                // we only need initial values, stop simulation run
                if (now > 1ns)
                {
                    receiver->Stop();
                }
            }
        );

        EXPECT_TRUE(testHarness.Run(30s)) << "TestHarness timeout reached!";
    }
protected:
    ib::cfg::Config ibConfig;

    std::vector<Topic> topics;
};
    
// NB this is disabled due to flaky FastRTPS behavior 
TEST_F(IoMessageITest, DISABLED_receive_init_values_fastrtps)
{
    RunTest(ib::cfg::Middleware::FastRTPS);
}

TEST_F(IoMessageITest, receive_init_values_vasio)
{
    RunTest(ib::cfg::Middleware::VAsio);
}

} // anonymous namespace
