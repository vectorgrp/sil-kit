// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <sstream>
#include <thread>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/util/serdes/sil/Serialization.hpp"

using namespace ib::mw;
using namespace ib::sim::data;
using namespace std::chrono_literals;


std::ostream& operator<<(std::ostream& os, const std::map<std::string, std::string>& m)
{
    os << "{ ";
    for (auto&& kvp : m)
        os << "{ " << kvp.first << ", " << kvp.second << " } "; 
    os << "}";

    return os;
}

void PublishMessage(IDataPublisher* publisher, std::string msg)
{
    static auto msgIdx = 0;

    std::stringstream messageBuilder;
    messageBuilder << msg << " LocalMsgId=" << msgIdx++;
    auto message = messageBuilder.str();

    std::cout << "<< Send DataMessageEvent with data=" << message << std::endl;

    ib::util::serdes::sil::Serializer serializer;
    serializer.Serialize(message);
    publisher->Publish(serializer.ReleaseBuffer());
}

void DefaultDataHandler(IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent)
{
    ib::util::serdes::sil::Deserializer deserializer(dataMessageEvent.data);
    const auto message = deserializer.Deserialize<std::string>();
    std::cout << ">> [DefaultDataHandler] Received new Message: with data=\""
              << message << "\"" << std::endl;
}

void SpecificDataHandlerForPub1(IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent)
{
    ib::util::serdes::sil::Deserializer deserializer(dataMessageEvent.data);
    const auto message = deserializer.Deserialize<std::string>();
    std::cout << ">> [SpecificDataHandlerForPublisher1] Received new Message: with data=\""
              << message << std::endl;
}

void SpecificDataHandlerForPub2(IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent)
{
    ib::util::serdes::sil::Deserializer deserializer(dataMessageEvent.data);
    const auto message = deserializer.Deserialize<std::string>();
    std::cout << ">> [SpecificDataHandlerForPublisher2] Received new Message: with data=\""
              << message << "\"" << std::endl;
}

void NewDataSource(IDataSubscriber* /*subscriber*/, const NewDataPublisherEvent& dataSource)
{
    std::cout << ">> New data source: topic=" << dataSource.topic << " mediaType=" << dataSource.mediaType
              << " labels=" << dataSource.labels << "" << std::endl;
}

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <ParticipantConfiguration.yaml|json> <ParticipantName> [domainId]" << std::endl;
        return -1;
    }

    try
    {
        std::string participantConfigurationFilename(argv[1]);
        std::string participantName(argv[2]);

        uint32_t domainId = 42;
        if (argc >= 4)
        {
            domainId = static_cast<uint32_t>(std::stoul(argv[3]));
        }

        auto participantConfiguration = ib::cfg::ParticipantConfigurationFromFile(participantConfigurationFilename);

        std::cout << "Creating participant '" << participantName << "' in domain " << domainId << std::endl;
        auto participant = ib::CreateParticipant(participantConfiguration, participantName, domainId);

        // Set an Init Handler
        auto* lifecycleService = participant->GetLifecycleService();
        auto* timeSyncService = lifecycleService->GetTimeSyncService();

        lifecycleService->SetCommunicationReadyHandler([&participantName]() {
            std::cout << "Communication ready for " << participantName << std::endl;
        });
        lifecycleService->SetStopHandler([]() {
            std::cout << "Stopping..." << std::endl;
        });

        lifecycleService->SetShutdownHandler([]() {
            std::cout << "Shutting down..." << std::endl;
        });

        const std::string mediaType{ ib::util::serdes::sil::MediaTypeData() };

        timeSyncService->SetPeriod(1s);
        if (participantName == "Publisher1")
        {
            std::map<std::string, std::string> labels{{"KeyA", "ValA"}, {"KeyB", "ValB"} };
            auto* publisher = participant->CreateDataPublisher("PubCtrl1", "Topic1", mediaType, labels, 0);

            timeSyncService->SetSimulationTask(
                [publisher](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    PublishMessage(publisher, "Pub1 on Topic1");
                    std::this_thread::sleep_for(1s);

            });
        }
        else if (participantName == "Publisher2")
        {
            std::map<std::string, std::string> labels{ {"KeyB", "ValB"}, {"KeyC", "ValC"} };
            auto* publisher = participant->CreateDataPublisher("PubCtrl1", "Topic1", mediaType, labels, 0);

            timeSyncService->SetSimulationTask(
                [publisher](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    PublishMessage(publisher, "Pub2 on Topic1");
                    std::this_thread::sleep_for(1s);
                });
        }
        else //if (participantName == "Subscriber")
        {
            auto* subscriber = participant->CreateDataSubscriber("SubCtrl1", "Topic1", mediaType, {}, DefaultDataHandler, NewDataSource);
           
            subscriber->AddExplicitDataMessageHandler(SpecificDataHandlerForPub1, mediaType,
                                                      {{"KeyA", ""}, {"KeyB", ""}});
            subscriber->AddExplicitDataMessageHandler(SpecificDataHandlerForPub2, mediaType, {{"KeyC", ""}});

            timeSyncService->SetSimulationTask(
                [](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                });
        }

        auto lifecycleFuture = lifecycleService->ExecuteLifecycleWithSyncTime(timeSyncService, true, true);
        auto finalState = lifecycleFuture.get();

        std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
    }
    catch (const ib::ConfigurationError& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -2;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -3;
    }

    return 0;
}