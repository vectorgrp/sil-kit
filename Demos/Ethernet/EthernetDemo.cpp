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

std::vector<uint8_t> CreateRawFrame(const eth::EthernetMac& destinationAddress, const eth::EthernetMac& sourceAddress,
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

void FrameTransmitHandler(eth::IEthernetController* /*controller*/, const eth::EthernetFrameTransmitEvent& frameTransmitEvent)
{
    if (frameTransmitEvent.status == eth::EthernetTransmitStatus::Transmitted)
    {
        std::cout << ">> ACK for Ethernet frame with transmitId=" << frameTransmitEvent.transmitId << std::endl;
    }
    else
    {
        std::cout << ">> NACK for Ethernet frame with transmitId=" << frameTransmitEvent.transmitId;
        switch (frameTransmitEvent.status)
        {
        case eth::EthernetTransmitStatus::Transmitted:
            break;
        case eth::EthernetTransmitStatus::InvalidFrameFormat:
            std::cout << ": InvalidFrameFormat";
            break;
        case eth::EthernetTransmitStatus::ControllerInactive:
            std::cout << ": ControllerInactive";
            break;
        case eth::EthernetTransmitStatus::LinkDown:
            std::cout << ": LinkDown";
            break;
        case eth::EthernetTransmitStatus::Dropped:
            std::cout << ": Dropped";
            break;
        case eth::EthernetTransmitStatus::DuplicatedTransmitId:
            std::cout << ": DuplicatedTransmitId";
            break;
        }

        std::cout << std::endl;
    }
}

void FrameHandler(eth::IEthernetController* /*controller*/, const eth::EthernetFrameEvent& frameEvent)
{
    auto rawFrame = frameEvent.ethFrame.RawFrame();
    auto payload = GetPayloadStringFromRawFrame(rawFrame);
    std::cout << ">> Ethernet frame: \""
              << payload
              << "\"" << std::endl;
}

void SendFrame(eth::IEthernetController* controller, const eth::EthernetMac& from, const eth::EthernetMac& to)
{
    static int frameId = 0;
    std::stringstream stream;
    stream << "Hello from Ethernet writer! (frameId =" << frameId++<< ")"
              "----------------------------------------------------"; // ensure that the payload is long enough to constitute a valid Ethernet frame

    auto payloadString = stream.str();
    std::vector<uint8_t> payload(payloadString.size() + 1);
    memcpy(payload.data(), payloadString.c_str(), payloadString.size() + 1);

    std::vector<uint8_t> rawFrame = CreateRawFrame(to, from, payload);

    eth::EthernetFrame frame;
    frame.SetRawFrame(rawFrame);
    
    auto transmitId = controller->SendFrame(std::move(frame));
    std::cout << "<< ETH Frame sent with transmitId=" << transmitId << std::endl;
}

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

int main(int argc, char** argv)
{
    ib::sim::eth::EthernetMac WriterMacAddr = {0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC1};
    //ib::sim::eth::EthernetMac ReaderMacAddr = {0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC2};
    ib::sim::eth::EthernetMac BroadcastMacAddr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

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
        auto* ethernetController = participant->CreateEthernetController("Eth1");

        ethernetController->AddFrameHandler(&FrameHandler);
        ethernetController->AddFrameTransmitHandler(&FrameTransmitHandler);

        if (runSync)
        {
            auto* participantController = participant->GetParticipantController();
            // Set an Init Handler
            participantController->SetInitHandler([&participantName, ethernetController](auto /*initCmd*/) {
                std::cout << "Initializing " << participantName << std::endl;
                ethernetController->Activate();
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
                    [ethernetController, WriterMacAddr, destinationAddress = BroadcastMacAddr](
                        std::chrono::nanoseconds now,
                                                                    std::chrono::nanoseconds /*duration*/) {
                        std::cout << "now=" << std::chrono::duration_cast<std::chrono::milliseconds>(now).count()
                                  << "ms" << std::endl;
                        SendFrame(ethernetController, WriterMacAddr, destinationAddress);
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
                        SendFrame(ethernetController, WriterMacAddr, BroadcastMacAddr);
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
