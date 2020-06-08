// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <future>
#include <thread>
#include <atomic>
#include <functional>

#include <iostream>

#include "CreateComAdapter.hpp"
#include "VAsioRegistry.hpp"

#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/util/functional.hpp"


#include "gtest/gtest.h"

#include "GetTestPid.hpp"

#include "IntegrationTestUtils.hpp"

namespace {

using namespace std::chrono_literals;
using namespace ib::mw;
using namespace ib::cfg;
using namespace ib::sim::eth;

using IntegrationTestUtils::Barrier;

class DummySystemController
{
public:
    DummySystemController() = delete;
    DummySystemController(const ib::cfg::Config& cfg, IComAdapterInternal* comAdapter)
        : _cfg{cfg}
        , _comAdapter{comAdapter}
    {
        _controller = _comAdapter->GetSystemController();
        _monitor = _comAdapter->GetSystemMonitor();

        _monitor->RegisterSystemStateHandler(
            std::bind(&DummySystemController::OnSystemStateChanged, this, std::placeholders::_1)
        );
        _monitor->RegisterParticipantStatusHandler(
            std::bind(&DummySystemController::OnParticipantStatusChanged, this, std::placeholders::_1)
        );
    }

    void InitializeAll()
    {
        for (const auto& participant : _cfg.simulationSetup.participants)
        {
            if (!participant.participantController)
                continue;

            std::cout << "Sending ParticipantCommand::Init to participant \"" 
                << participant.name << "\"" << std::endl;
            _controller->Initialize(participant.id);
        }
    }

    void OnParticipantStatusChanged(sync::ParticipantStatus status)
    {
        if (status.state == sync::ParticipantState::Stopped)
        {
            std::cout << "System: participant " << status.participantName << " stopped!" << std::endl;
            _controller->Stop();
        }
    }

    void OnSystemStateChanged(sync::SystemState state)
    {
        switch (state)
        {
        case sync::SystemState::Idle:
            std::cout << "System Idle" << std::endl;
            InitializeAll();
            return;
        case sync::SystemState::Initialized:
            std::cout << "System Initialized" << std::endl;
            _controller->Run();
            return;
        case sync::SystemState::Running:
            std::cout << "System Running" << std::endl;
            return;
        case sync::SystemState::Stopped:
            std::cout << "System Shutdown" << std::endl;
            _controller->Shutdown();
            return;
        default:
            return;
        }
    }

private:
    const ib::cfg::Config& _cfg;

    sync::ISystemController* _controller;
    sync::ISystemMonitor* _monitor;
    IComAdapterInternal* _comAdapter;
};

auto makeConfigWithParticipant() -> ib::cfg::Config
{
    ib::cfg::ConfigBuilder builder{"TestConfig"};
    auto &&setup = builder.SimulationSetup();
    setup.AddParticipant("EthWriter")
        ->AddEthernet("ETH1").WithLink("LINK1")
        ->AsSyncMaster()
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

bool hasParticipantController(const ib::cfg::Config& config)
{
    for (const auto& part : config.simulationSetup.participants)
    {
        if (part.participantController.has_value()) return true;
    }
    return false;
}

class TimeProviderITest: public testing::Test
{
protected:

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
            controller->SendFrame(msg.ethFrame);
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
                std::cout << " -> RX msg time_abs: " << msg.timestamp.count()
                    << " time_dif: " << D.count() << std::endl;
                lastTime = msg.timestamp;

                receiveCount++;
                if ((participantName == "EthReader1") && (receiveCount == numMessages))
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
        auto* sysctrl = comAdapter->GetSystemController();

        parti->SetSimulationTask([this, controller, &msg, &numSent, sysctrl](auto, auto) {
            if(numSent++ < numMessages)
            {
                //let controller use its Time Provider to add timestamps
                controller->SendFrame(msg.ethFrame);
                std::cout << "<- simTask TX num=" << numSent << std::endl;
                std::this_thread::sleep_for(40ms);
            }
        });

        parti->SetInitHandler([](sync::ParticipantCommand command) {
            std::cout << "EthWriter@" << command << std::endl;
        });

        parti->SetStopHandler([&sysctrl]() {
            std::cout << "EthWriter stop" << std::endl;
            sysctrl->Shutdown();
        });

        parti->SetShutdownHandler([]() {
            std::cout << "EthWriter shutdown" << std::endl;
        });


        //use the systemcontroller to set all participants into running state
        _sysCtrl = std::make_unique<DummySystemController>(ibConfig, comAdapter.get());

        auto done = parti->RunAsync();

        barrier->Enter(); //sync

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
                    if (receiveCount > 0)
                    {
                        // The participant time providers' time advances at tick
                        // period rate, starting with 0ns.
                        EXPECT_EQ(D, ibConfig.simulationSetup.timeSync.tickPeriod)
                            << "the participant time provider should have the exact"
                            << " tick period of the simulation.";
                    }

                    std::cout << " -> RX msg time_abs: " << msg.timestamp.count()
                        << " time_dif: " << D.count() << std::endl;
                    lastTime = msg.timestamp;

                }
                receiveCount++;
        });
        auto* sysctl = comAdapter->GetSystemController();
        auto* parti = comAdapter->GetParticipantController();
        parti->SetSimulationTask([this, &participantName, &receiveCount, sysctl](auto , auto) {
            if (receiveCount == numMessages)
            {
                std::cout << participantName << " stopping" << std::endl;
                sysctl->Stop();
            }
        });

        parti->SetStopHandler([sysctl, &participantName]() {
            std::cout << participantName << " shutdown" << std::endl;
            sysctl->Shutdown();
        });

        parti->SetInitHandler([&participantName](sync::ParticipantCommand command) {
            std::cout << participantName << "@"  << command << std::endl;
        });

        parti->SetShutdownHandler([&participantName]() {
            std::cout << participantName << " shutdown" << std::endl;
        });

        auto done = parti->RunAsync();

        barrier->Enter();

        ASSERT_EQ(done.wait_for(waitTime), std::future_status::ready);
    }
protected:
    std::unique_ptr<DummySystemController> _sysCtrl{};
};

TEST_F(ParticipantTimeProviderITest, test_participant_timestamps_fastrtps)
{
    ibConfig = makeConfigWithParticipant();
    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::FastRTPS;

    ExecuteTest();
}

TEST_F(ParticipantTimeProviderITest, test_participant_timestamps_vasio)
{
    ibConfig = makeConfigWithParticipant();
    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::VAsio;

    auto registry = std::make_unique<VAsioRegistry>(ibConfig);
    registry->ProvideDomain(domainId);

    ExecuteTest();
}
} // anonymous namespace
