// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <sstream>
#include <thread>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"

using namespace ib;
using namespace ib::mw;
using namespace ib::mw::sync;
using namespace ib::sim::generic;
using namespace std::chrono_literals;

class IbController
{
public:
    IbController(IComAdapter* comAdapter, cfg::Config ibConfig)
        : ibConfig{std::move(ibConfig)}
    {
        _systemController = comAdapter->GetSystemController();
        _monitor = comAdapter->GetSystemMonitor();

        _monitor->RegisterSystemStateHandler(
            [this](SystemState newState) {
            this->OnSystemStateChanged(newState);
        });
    }

    void OnSystemStateChanged(SystemState newState)
    {
        switch (newState)
        {
        case SystemState::Idle:
            InitializeAllParticipants();
            return;
        case SystemState::Initialized:
            _systemController->Run();
            return;
        case SystemState::Stopping:
            return;

        case SystemState::Stopped:
            _systemController->Shutdown();
            return;

        case SystemState::Shutdown:
            return;
        }
    }

    void InitializeAllParticipants()
    {
        for (auto&& participant : ibConfig.simulationSetup.participants)
        {
            if (!participant.participantController)
                continue;

            _systemController->Initialize(participant.id);
        }
    }

public:
    // ----------------------------------------
    //  Public Members
    ib::cfg::Config ibConfig;

    ISystemController* _systemController{nullptr};
    ISystemMonitor* _monitor{nullptr};
};

class IbPublisher
{
public:
    IbPublisher(IComAdapter* comAdapter, std::vector<std::string> testMessages)
        : _testMessages{std::move(testMessages)}
        , _comAdapter{comAdapter}
    {
    }

    void SetupParticipant(const std::string participantName)
    {
        auto&& participantController = _comAdapter->GetParticipantController();

        auto* publisher = _comAdapter->CreateGenericPublisher("GenericData");

        participantController->SetSimulationTask(
            [this, publisher](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {

            if (_messageIndex < _testMessages.size())
            {
                PublishMessage(publisher);
                _messageIndex++;
            }

        });

        participantController->Run();
    }

    void PublishMessage(IGenericPublisher* publisher)
    {
        auto testMessage = _testMessages.at(_messageIndex);
        std::vector<uint8_t> testData{testMessage.begin(), testMessage.end()};

        publisher->Publish(std::move(testData));
    }

public:
    // ----------------------------------------
    //  Public Members
    std::vector<std::string> _testMessages;
    uint32_t _messageIndex{0};

    IComAdapter* _comAdapter{nullptr};
};

class IbSubscriber
{
public:
    IbSubscriber(IComAdapter* comAdapter, std::vector<std::string> testMessages)
        : _testMessages{std::move(testMessages)}
        , _comAdapter{comAdapter}
    {
        _systemController = _comAdapter->GetSystemController();
    }

    void SetupParticipant(const std::string participantName)
    {
        auto&& participantController = _comAdapter->GetParticipantController();

        auto* subscriber = _comAdapter->CreateGenericSubscriber("GenericData");
        subscriber->SetReceiveMessageHandler(
            [this](IGenericSubscriber* subscriber, const std::vector<uint8_t>& data) {
            ReceiveMessage(subscriber, data);
        });

        participantController->SetSimulationTask(
            [this, subscriber](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {

            _tickCount++;
            if (_tickCount > _messageIndex + 3)
            {
                std::cout << "Test unsuccessful: System is still running. "
                          << "The subscriber should have called stopped after receiving the last message." << std::endl;
                auto* systemController = _comAdapter->GetSystemController();
                systemController->Stop();
            }

        });

        participantController->Run();
    }

    void ReceiveMessage(IGenericSubscriber* subscriber, const std::vector<uint8_t>& data)
    {
        std::string receivedMessage{data.begin(), data.end()};

        // Exactly one message should be sent and received in the same simulation tick.
        // _tickCount is incremented in each simulation tick, so _messageIndex should
        // be equal to _tickCount or one smaller if _tickCount has already been incremented.
        if (!(_messageIndex == _tickCount || _messageIndex + 1 == _tickCount))
        {
            _errorCount++;
            std::cout << "Test unsuccessful: The received message did not arrive in the expected tick period (strict mode)." << std::endl;
        }
        if (receivedMessage != _testMessages.at(_messageIndex))
        {
            _errorCount++;
            std::cout << "Test unsuccessful: The received message did not match the sent test message." << std::endl;
        }

        if (_messageIndex < _testMessages.size())
        {
            _messageIndex++;
        }
        else
        {
            _systemController->Stop();
        }
    }

public:
    // ----------------------------------------
    //  Public Members
    std::vector<std::string> _testMessages;
    uint32_t _messageIndex{0};
    uint32_t _errorCount{0};
    uint32_t _tickCount{0};

    IComAdapter* _comAdapter{nullptr};
    ISystemController* _systemController{nullptr};
};

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

int main(int argc, char** argv) try
{
    if (argc < 2)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <IbConfig.json> [domainId]" << std::endl;
        return -1;
    }

    std::string configFilename(argv[1]);
    std::string participantNamePublisher("Publisher");
    std::string participantNameSubscriber("Subscriber");

    uint32_t domainId = 42;
    if (argc >= 3)
    {
        domainId = static_cast<uint32_t>(std::stoul(argv[2]));
    }

    auto ibConfig = ib::cfg::Config::FromJsonFile(configFilename);
    auto tickPeriod = ibConfig.simulationSetup.timeSync.tickPeriod;

    auto comAdapterPublisher = ib::CreateFastRtpsComAdapter(ibConfig, participantNamePublisher, domainId);
    auto comAdapterSubscriber = ib::CreateFastRtpsComAdapter(ibConfig, participantNameSubscriber, domainId);
    auto comAdapterController = ib::CreateFastRtpsComAdapter(ibConfig, "SystemController", domainId);

    IbController ibController(comAdapterController.get(), ibConfig);

    std::vector<std::string> testData
    {
        "This",
        "is a",
        "unique",
        "test vector",
        "of generic",
        "messages",
        "full of",
        "different",
        "message",
        "strings! Test End Reached."
    };
    IbPublisher publisher(comAdapterPublisher.get(), testData);
    IbSubscriber subscriber(comAdapterSubscriber.get(), testData);

    std::thread t1(&IbPublisher::SetupParticipant, &publisher, participantNamePublisher);
    std::thread t2(&IbSubscriber::SetupParticipant, &subscriber, participantNameSubscriber);

    t1.join();
    t2.join();

    std::cout << "---------------------------------------------------------------------\n";
    std::cout << "Test Run Finished: "
              << unsigned(publisher._messageIndex)  << " messages sent, "
              << unsigned(subscriber._messageIndex) << " messages received, "
              << unsigned(subscriber._errorCount)   << " test cases (messages) failed."
              << std::endl;
    std::cout << "---------------------------------------------------------------------" << std::endl;

    return 0;
}
catch (const ib::cfg::Misconfiguration& error)
{
    std::cerr << "Invalid configuration: " << error.what() << std::endl;
    return -2;
}
catch (const std::exception& error)
{
    std::cerr << "Something went wrong: " << error.what() << std::endl;
    return -3;
}
