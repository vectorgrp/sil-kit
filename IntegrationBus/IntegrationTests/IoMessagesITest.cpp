// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "CreateComAdapter.hpp"
#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"

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
        : domainId{static_cast<uint32_t>(GetTestPid())}
        , topics(4)
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
        
        pubComAdapter = CreateFastRtpsComAdapterImpl(ibConfig, "Sender");
        pubComAdapter->joinIbDomain(domainId);

        subComAdapter = CreateFastRtpsComAdapterImpl(ibConfig, "Receiver");
        subComAdapter->joinIbDomain(domainId);
    }


    void Subscribe()
    {
        auto dio1 = subComAdapter->CreateDigitalIn(topics[0].name);
        SetExpectation(topics[0], dio1 , true);
        auto dio2 = subComAdapter->CreateDigitalIn(topics[1].name);
        SetExpectation(topics[1], dio2, false);
        auto aio1 = subComAdapter->CreateAnalogIn(topics[2].name);
        SetExpectation(topics[2], aio1, 5.0);
        auto aio2 = subComAdapter->CreateAnalogIn(topics[3].name);
        SetExpectation(topics[3], aio2, 17.3);
    }

    void Publish()
    {
        pubComAdapter->CreateDigitalOut(topics[0].name);
        pubComAdapter->CreateDigitalOut(topics[1].name);
        pubComAdapter->CreateAnalogOut(topics[2].name);
        pubComAdapter->CreateAnalogOut(topics[3].name);
    }

    struct Topic
    {
        std::string name;
        std::promise<bool> testOK;
    };


    template<class InPortT>
    static void SetExpectation(Topic& topic, InPortT* port, const typename InPortT::ValueType& expectedValue)
    {
        port->RegisterHandler(
            [&topic, expectedValue](InPortT* /*port*/, const typename InPortT::ValueType& data)
            {
                EXPECT_EQ(data, expectedValue);
                topic.testOK.set_value(data == expectedValue);
            }
        );
    }

protected:
    const uint32_t domainId;
    ib::cfg::Config ibConfig;

    std::vector<Topic> topics;

    std::unique_ptr<IComAdapterInternal> pubComAdapter;
    std::unique_ptr<IComAdapterInternal> subComAdapter;
};
    
TEST_F(IoMessageITest, receive_init_values)
{
    Subscribe();

    std::thread publishThread{[this]() { this->Publish(); }};

    for (auto&& topic : topics)
    {
        auto&& testOK = topic.testOK.get_future();
        auto futureStatus = testOK.wait_for(5s);
        ASSERT_EQ(futureStatus, std::future_status::ready);
        EXPECT_TRUE(testOK.get());
    }
    
    publishThread.join();
}

} // anonymous namespace
