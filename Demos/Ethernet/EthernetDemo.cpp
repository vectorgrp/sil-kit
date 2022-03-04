// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <cstring>

#include <algorithm>
#include <sstream>
#include <iostream>
#include <string>
#include <thread>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"

using namespace ib::mw;
using namespace ib::sim;

using namespace std::chrono_literals;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

void EthAckCallback(eth::IEthController* /*controller*/, const eth::EthTransmitAcknowledge& ack)
{
    if (ack.status == eth::EthTransmitStatus::Transmitted)
    {
        std::cout << ">> ACK for ETH Message with transmitId=" << ack.transmitId << std::endl;
    }
    else
    {
        std::cout << ">> NACK for ETH Message with transmitId=" << ack.transmitId;
        switch (ack.status)
        {
        case eth::EthTransmitStatus::Transmitted:
            break;
        case eth::EthTransmitStatus::InvalidFrameFormat:
            std::cout << ": InvalidFrameFormat";
            break;
        case eth::EthTransmitStatus::ControllerInactive:
            std::cout << ": ControllerInactive";
            break;
        case eth::EthTransmitStatus::LinkDown:
            std::cout << ": LinkDown";
            break;
        case eth::EthTransmitStatus::Dropped:
            std::cout << ": Dropped";
            break;
        case eth::EthTransmitStatus::DuplicatedTransmitId:
            std::cout << ": DuplicatedTransmitId";
            break;
        }

        std::cout << std::endl;
    }
}

void ReceiveEthMessage(eth::IEthController* /*controller*/, const eth::EthMessage& msg)
{
    auto tci = msg.ethFrame.GetVlanTag();
    std::cout << ">> ETH Message: \""
              << reinterpret_cast<const char*>(msg.ethFrame.GetPayload().data())
              << "  TCI={pcp=" << static_cast<uint16_t>(tci.pcp)
              << ",dei=" << static_cast<uint16_t>(tci.dei)
              << ",vid=" << tci.vid << "}"
              << "\"" << std::endl;
}

void SendMessage(eth::IEthController* controller, const eth::EthMac& from, const eth::EthMac& to)
{
    eth::EthTagControlInformation tci;
    tci.pcp = 3;
    tci.dei = 0;
    tci.vid = 1;

    static int msgId = 0;
    std::stringstream stream;
    stream << "Hello from ethernet writer! (msgId=" << msgId++<< ")"
              "----------------------------------------------------"; // ensure that the payload is long enough to constitute a valid ethernet frame

    auto hello = stream.str();
    std::vector<uint8_t> payload(hello.size() + 1);
    memcpy(payload.data(), hello.c_str(), hello.size() + 1);

    eth::EthFrame frame;
    frame.SetSourceMac(from);
    frame.SetDestinationMac(to);
    frame.SetVlanTag(tci);
    frame.SetPayload(payload);

    auto transmitId = controller->SendFrame(std::move(frame));
    std::cout << "<< ETH Frame sent with transmitId=" << transmitId << std::endl;
}

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

int main(int argc, char** argv)
{
    ib::sim::eth::EthMac WriterMacAddr = {0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC1};
    ib::sim::eth::EthMac ReaderMacAddr = {0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC2};

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

        std::cout << "Creating ComAdapter for Participant=" << participantName << " in Domain " << domainId << std::endl;
        auto participant = ib::CreateSimulationParticipant(participantConfiguration, participantName, domainId, true);
        auto* ethController = participant->CreateEthController("ETH0");
        auto* participantController = participant->GetParticipantController();

        ethController->RegisterReceiveMessageHandler(&ReceiveEthMessage);
        ethController->RegisterMessageAckHandler(&EthAckCallback);

        // Set an Init Handler
        participantController->SetInitHandler([&participantName, ethController](auto /*initCmd*/) {

            std::cout << "Initializing " << participantName << std::endl;
            ethController->Activate();

        });

        // Set a Stop Handler
        participantController->SetStopHandler([]() {

            std::cout << "Stopping..." << std::endl;

        });

        // Set a Shutdown Handler
        participantController->SetShutdownHandler([]() {

            std::cout << "Shutting down..." << std::endl;

        });

        participantController->SetPeriod(1ms);
        if (participantName == "EthernetWriter")
        {
            participantController->SetSimulationTask(
                [ethController, &WriterMacAddr, &ReaderMacAddr](std::chrono::nanoseconds now,
                                                                  std::chrono::nanoseconds /*duration*/) {

                    std::cout << "now=" << std::chrono::duration_cast<std::chrono::milliseconds>(now).count() << "ms" << std::endl;
                    SendMessage(ethController, WriterMacAddr, ReaderMacAddr);
                    std::this_thread::sleep_for(1s);

            });
        }
        else
        {
            participantController->SetSimulationTask(
                [](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {

                    std::cout << "now=" << std::chrono::duration_cast<std::chrono::milliseconds>(now).count() << "ms" << std::endl;
                    std::this_thread::sleep_for(1s);

            });
        }

        //auto finalStateFuture = participantController->RunAsync();
        //auto finalState = finalStateFuture.get();

        auto finalState = participantController->Run();

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
