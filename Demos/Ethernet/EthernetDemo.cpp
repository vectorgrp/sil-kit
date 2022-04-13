// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

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

std::vector<uint8_t> CreateRawFrame(const eth::EthMac& destinationAddress, const eth::EthMac& sourceAddress,
                                    const std::vector<uint8_t>& payload)
{
    const std::vector<uint8_t> etherType = {0x00, 0x00};
    const std::vector<uint8_t> dummyCrc = {0x0, 0x0, 0x0, 0x0};

    std::vector<uint8_t> rawFrame;
    rawFrame.insert(rawFrame.end(), destinationAddress.begin(), destinationAddress.end()); // destination
    rawFrame.insert(rawFrame.end(), sourceAddress.begin(), sourceAddress.end()); // source
    rawFrame.insert(rawFrame.end(), etherType.begin(), etherType.end()); // EtherType -- no need to check for VLAN-Tag
    rawFrame.insert(rawFrame.end(), payload.begin(), payload.end()); // payload
    rawFrame.insert(rawFrame.end(), dummyCrc.begin(), dummyCrc.end()); // dummy CRC

    // make sure that the result is a valid Ethernet frame
    if (rawFrame.size() < 64)
    {
        auto fillSize = static_cast<size_t>(64 - rawFrame.size());
        std::vector<uint8_t> filler(fillSize, 0);
        rawFrame.insert(rawFrame.end(), filler.begin(), filler.end()); // expand to valid frame
    }
    return rawFrame;
}

std::string GetPayloadStringFromRawFrame(const std::vector<uint8_t>& rawFrame)
{
    std::vector<uint8_t> payload;
    payload.insert(payload.end(), rawFrame.begin() + 14 /*destination[6]+source[6]+ethType[2]*/,
                   rawFrame.end() - 4); // CRC
    std::string payloadString(payload.begin(), payload.end());
    return payloadString;
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
    auto rawFrame = msg.ethFrame.RawFrame();
    auto payload = GetPayloadStringFromRawFrame(rawFrame);
    std::cout << ">> ETH Message: \""
              << payload
              << "\"" << std::endl;
}

void SendMessage(eth::IEthController* controller, const eth::EthMac& from, const eth::EthMac& to)
{
    static int msgId = 0;
    std::stringstream stream;
    stream << "Hello from Ethernet writer! (msgId=" << msgId++<< ")"
              "----------------------------------------------------"; // ensure that the payload is long enough to constitute a valid Ethernet frame

    auto payloadString = stream.str();
    std::vector<uint8_t> payload(payloadString.size() + 1);
    memcpy(payload.data(), payloadString.c_str(), payloadString.size() + 1);

    std::vector<uint8_t> rawFrame = CreateRawFrame(to, from, payload);

    eth::EthFrame frame;
    frame.SetRawFrame(rawFrame);

    auto transmitId = controller->SendFrame(std::move(frame));
    std::cout << "<< ETH Frame sent with transmitId=" << transmitId << std::endl;
}

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

int main(int argc, char** argv)
{
    ib::sim::eth::EthMac WriterMacAddr = {0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC1};
    //ib::sim::eth::EthMac ReaderMacAddr = {0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC2};
    ib::sim::eth::EthMac BroadcastMacAddr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0]
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> [domainId] [--async]" << std::endl
                  << "Use \"EthernetWriter\" or \"EthernetReader\" as <ParticipantName>." << std::endl;
        return -1;
    }

    if (argc > 5)
    {
        std::cerr << "Too many arguments! Start demo with: " << argv[0]
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> [domainId] [--async]" << std::endl
                  << "Use \"EthernetWriter\" or \"EthernetReader\" as <ParticipantName>." << std::endl;
        return -1;
    }

    try
    {
        std::string participantConfigurationFilename(argv[1]);
        std::string participantName(argv[2]);

        uint32_t domainId = 42;

        bool runSync = true;

        // skip argv[0] and collect all arguments
        std::vector<std::string> args;
        std::copy((argv + 3), (argv + argc), std::back_inserter(args));

        for (auto arg : args)
        {
            if (arg == "--async")
            {
                runSync = false;
            }
            else
            {
                try
                {
                    domainId = static_cast<uint32_t>(std::stoul(arg));
                }
                catch (...)
                {
                    std::cout << "Error: expected a numeric argument for [domainId] but got '" << arg << "'"
                              << std::endl;
                    return -1;
                }
            }
        }

        auto participantConfiguration = ib::cfg::ParticipantConfigurationFromFile(participantConfigurationFilename);

        std::cout << "Creating participant '" << participantName << "' in domain " << domainId << std::endl;
        auto participant = ib::CreateParticipant(participantConfiguration, participantName, domainId, runSync);
        auto* ethController = participant->CreateEthController("Eth1");

        ethController->RegisterReceiveMessageHandler(&ReceiveEthMessage);
        ethController->RegisterMessageAckHandler(&EthAckCallback);

        if (runSync)
        {
            auto* participantController = participant->GetParticipantController();
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
                    [ethController, WriterMacAddr, destinationAddress = BroadcastMacAddr](
                        std::chrono::nanoseconds now,
                                                                    std::chrono::nanoseconds /*duration*/) {
                        std::cout << "now=" << std::chrono::duration_cast<std::chrono::milliseconds>(now).count()
                                  << "ms" << std::endl;
                        SendMessage(ethController, WriterMacAddr, destinationAddress);
                        std::this_thread::sleep_for(1s);
                    });
            }
            else
            {
                participantController->SetSimulationTask(
                    [](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                        std::cout << "now=" << std::chrono::duration_cast<std::chrono::milliseconds>(now).count()
                                  << "ms" << std::endl;
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
        else
        {
            bool isStopped = false;
            std::thread workerThread;

            if (participantName == "EthernetWriter")
            {
                workerThread = std::thread{[&]() {
                    while (!isStopped)
                    {
                        SendMessage(ethController, WriterMacAddr, BroadcastMacAddr);
                        std::this_thread::sleep_for(1s);
                    }
                }};
            }
            else
            {
                workerThread = std::thread{[&]() {
                    while (!isStopped)
                    {
                        std::this_thread::sleep_for(1s);
                    }
                }};
            }

            std::cout << "Press enter to stop the process..." << std::endl;
            std::cin.ignore();
            isStopped = true;
            workerThread.join();
        }

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
