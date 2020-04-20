// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>
#include <atomic>

#include <iostream>

#include "CreateComAdapter.hpp"
#include "VAsioRegistry.hpp"

#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/util/functional.hpp"


#include "gtest/gtest.h"

#include "GetTestPid.hpp"

namespace {

using namespace std::chrono_literals;
using namespace ib::mw;
using namespace ib::cfg;
using namespace ib::sim::eth;

struct Barrier
{
    std::mutex mx;
    std::condition_variable cv;
    std::atomic_uint expected{0};
    std::atomic_uint have{0};
    std::chrono::seconds timeout{1};
    
    Barrier(Barrier&) = delete;

    Barrier(unsigned expectedEntries, std::chrono::seconds timeout)
        : expected{expectedEntries}
        , timeout{timeout}
    {}
    
    void Enter()
    {
        std::unique_lock<decltype(mx)> lock(mx);
        have++;
        if (have >= expected)
        {
            lock.unlock();
            cv.notify_all();
        }
        else
        {
            auto ok = cv.wait_for(lock, timeout, [this] {return have == expected; });
            if (!ok) { std::cout <<  "Barrier: timeout!" << std::endl; }
        }
    }
};

class TimeProviderITest: public testing::Test
{
protected:
    static auto MakeConfigWithParticipant() -> ib::cfg::Config
    {
        ib::cfg::ConfigBuilder builder{"TestConfig"};
        auto &&setup = builder.SimulationSetup();
        setup.AddParticipant("EthWriter")
            ->AddEthernet("ETH1").WithLink("LINK1")
            ->AddParticipantController().WithSyncType(SyncType::DiscreteTime);
        setup.AddParticipant("EthReader1")
            ->AddEthernet("ETH1").WithLink("LINK1")
            ->AddParticipantController().WithSyncType(SyncType::DiscreteTime);
        setup.AddParticipant("EthReader2")
            ->AddEthernet("ETH1").WithLink("LINK1")
            ->AddParticipantController().WithSyncType(SyncType::DiscreteTime);
        setup.ConfigureTimeSync().WithTickPeriod(1ms);
        return builder.Build();
    }

    static bool hasParticipantController(const ib::cfg::Config& config)
    {
        for (const auto& part : config.simulationSetup.participants)
        {
            if (part.participantController.has_value()) return true;
        }
        return false;
    }

    TimeProviderITest()
    {
        domainId = static_cast<uint32_t>(GetTestPid());

        ib::cfg::ConfigBuilder builder{"TestConfig"};
        auto &&setup = builder.SimulationSetup();
        setup.AddParticipant("EthWriter")
            ->AddEthernet("ETH1").WithLink("LINK1");
        setup.AddParticipant("EthReader1")
            ->AddEthernet("ETH1").WithLink("LINK1");
        setup.AddParticipant("EthReader2")
            ->AddEthernet("ETH1").WithLink("LINK1");

        ibConfig = builder.Build();
    }

    virtual void Sender()
    {
        ASSERT_FALSE(hasParticipantController(ibConfig));

        auto comAdapter = CreateComAdapterImpl(ibConfig, "EthWriter");
        comAdapter->joinIbDomain(domainId);

        std::atomic_uint64_t receiveCount{0};

        auto* controller = comAdapter->CreateEthController("ETH1");
        controller->RegisterMessageAckHandler(
            [this, &receiveCount](IEthController*, const EthTransmitAcknowledge& ) {
            receiveCount++;
        });

        const std::string message = "EthWriter TimeProvider Test!";
        EthMessage msg{};
        msg.ethFrame.SetDestinationMac(EthMac{ 0x12,0x23,0x45,0x67,0x89,0x9a });
        msg.ethFrame.SetSourceMac(EthMac{ 0x9a, 0x89, 0x67, 0x45, 0x23, 0x12});
        msg.ethFrame.SetEtherType(0x8100);
        msg.ethFrame.SetPayload(reinterpret_cast<const uint8_t*>(message.c_str()), message.size());

        barrier->Enter();

        for (auto i = 0u; i < numMessages; i++)
        {

            //let controller use its Time Provider to add timestamps
            controller->SendMessage(msg);
            std::this_thread::sleep_for(50ms);
        }

        ASSERT_EQ(isDone.wait_for(waitTime), std::future_status::ready)
            << "Sender receiveACKCount:" << receiveCount  
            << " numMessages: " << numMessages;
    }

    virtual void Receiver(const std::string& participantName)
    {
        ASSERT_FALSE(hasParticipantController(ibConfig));
        auto comAdapter = CreateComAdapterImpl(ibConfig, participantName);
        comAdapter->joinIbDomain(domainId);

        std::atomic_uint64_t receiveCount{0};
        std::chrono::nanoseconds lastTime{};

        auto* controller = comAdapter->CreateEthController("ETH1");
        controller->RegisterReceiveMessageHandler(
            [this, &receiveCount, &participantName, &lastTime](IEthController* , const EthMessage& msg) {
                // now >= lastTime
                EXPECT_GE(msg.timestamp, lastTime);
                auto D = msg.timestamp - lastTime;
                std::cout << " -> RX msg time_abs: " << msg.timestamp.count() << " time_dif: " << D.count() << std::endl;
                lastTime = msg.timestamp;

                receiveCount++;
                if (receiveCount == numMessages)
                    donePromise.set_value();
                
        });
        barrier->Enter();
        //block until all messages transmitted
        ASSERT_EQ(isDone.wait_for(waitTime), std::future_status::ready)
            << "Receiver " << participantName 
            <<" receiveCount: " << receiveCount 
            << " numMessages:" << numMessages;
    }

    void ExecuteTest()
    {
        isDone = donePromise.get_future();
        barrier = std::make_unique<Barrier>(3, waitTime);

        std::thread receiver1Thread{&TimeProviderITest::Receiver, this,"EthReader1"};
        std::thread receiver2Thread{&TimeProviderITest::Receiver, this,"EthReader2"};
        std::thread senderThread{&TimeProviderITest::Sender, this};

        senderThread.join();
        receiver1Thread.join();
        receiver2Thread.join();
    }
protected:
    uint32_t domainId;
    ib::cfg::Config ibConfig;

    std::unique_ptr<Barrier> barrier;
    std::promise<void> allReceived;
    std::shared_future<void> isAllReceived;

    std::promise<void> donePromise;
    std::shared_future<void> isDone;

    const uint32_t numMessages{7};
    const std::chrono::seconds waitTime{15};
};


TEST_F(TimeProviderITest, test_default_timestamps_fastrtps)
{
    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::FastRTPS;

    ExecuteTest();
}

TEST_F(TimeProviderITest, test_default_timestamps_vasio)
{
    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::VAsio;

    auto registry = std::make_unique<VAsioRegistry>(ibConfig);
    registry->ProvideDomain(domainId);

    ExecuteTest();
}

///////////////////////////////////////////////////////
// Check participant controllers time provider
///////////////////////////////////////////////////////
struct ParticipantTimeProviderITest : public TimeProviderITest
{
    // participant controller based tests with simulation tasks
    void Sender() override
    {
        ASSERT_TRUE(hasParticipantController(ibConfig));
        std::cout << " Sender with simtask init" << std::endl;
        auto comAdapter = CreateComAdapterImpl(ibConfig, "EthWriter");
        comAdapter->joinIbDomain(domainId);

        std::atomic_uint64_t receiveCount{0};

        auto* controller = comAdapter->CreateEthController("ETH1");
        controller->RegisterMessageAckHandler(
            [ &receiveCount](IEthController*, const EthTransmitAcknowledge& ) {
            ++receiveCount;
        });

        const std::string message = "EthWriter TimeProvider Test!";
        EthMessage msg{};
        msg.ethFrame.SetDestinationMac(EthMac{ 0x12,0x23,0x45,0x67,0x89,0x9a });
        msg.ethFrame.SetSourceMac(EthMac{ 0x9a, 0x89, 0x67, 0x45, 0x23, 0x12});
        msg.ethFrame.SetEtherType(0x8100);
        msg.ethFrame.SetPayload(reinterpret_cast<const uint8_t*>(message.c_str()), message.size());


        auto* parti = comAdapter->GetParticipantController();
        auto numSent = 0u;
        parti->SetSimulationTask([this, controller, &msg, &numSent](auto, auto) {
            if(numSent++ < numMessages)
            {
                //let controller use its Time Provider to add timestamps
                controller->SendMessage(msg);
                std::cout << "<- simTask TX" << std::endl;
                std::this_thread::sleep_for(100ms);
            }
        });
        auto done = parti->RunAsync();
        ASSERT_EQ(done.wait_for(waitTime), std::future_status::ready);
    }


    void Receiver(const std::string& participantName) override
    {
        ASSERT_TRUE(hasParticipantController(ibConfig));
        std::cout << " Receiver simtask init " << participantName << std::endl;
        auto comAdapter = CreateComAdapterImpl(ibConfig, participantName);
        comAdapter->joinIbDomain(domainId);

        std::atomic_uint64_t receiveCount{0};
        std::chrono::nanoseconds lastTime{};

        auto* controller = comAdapter->CreateEthController("ETH1");
        controller->RegisterReceiveMessageHandler(
            [this, &receiveCount, &participantName, &lastTime](IEthController* , const EthMessage& msg) {
                if (participantName == "EthReader1")
                {
                    // now >= lastTime
                    EXPECT_GE(msg.timestamp, lastTime);
                    auto D = msg.timestamp - lastTime;
                    std::cout << " -> RX msg time_abs: " << msg.timestamp.count()  << " time_dif: " << D.count() << std::endl;
                    lastTime = msg.timestamp;

                }
                receiveCount++;
        });
        auto* sysctl = comAdapter->GetSystemController();
        auto* parti = comAdapter->GetParticipantController();
        parti->SetSimulationTask([this, &receiveCount, sysctl](auto now, auto) {
            std::cout << "simtask now=" << now.count() << std::endl;
            if (receiveCount == numMessages)
            {
                sysctl->Stop();
            }
        });
        auto done = parti->RunAsync();
        ASSERT_EQ(done.wait_for(waitTime), std::future_status::ready);
    }
};

/* TODO not active yet:
TEST_F(ParticipantTimeProviderITest, test_participant_timestamps_fastrtps)
{
    ibConfig = MakeConfigWithParticipant();
    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::FastRTPS;

    ExecuteTest();
}

TEST_F(ParticipantTimeProviderITest, test_participant_timestamps_vasio)
{
    ibConfig = MakeConfigWithParticipant();
    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::VAsio;

    auto registry = std::make_unique<VAsioRegistry>(ibConfig);
    registry->ProvideDomain(domainId);

    ExecuteTest();
}
*/

} // anonymous namespace
