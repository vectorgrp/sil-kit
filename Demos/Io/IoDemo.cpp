// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
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

void ReceiveAnalogIoMessage(io::IAnalogInPort* ioPort, const io::AnalogIoMessage& msg)
{

    std::cout << ">> ANALOG  I/O Message on port " << ioPort->Config().name
              << " timestamp=" << msg.timestamp
              << " value=" << msg.value
              << std::endl;
}

void ReceiveDigitalIoMessage(io::IDigitalInPort* ioPort, const io::DigitalIoMessage& msg)
{
    std::cout << ">> DIGITAL I/O Message on port " << ioPort->Config().name
        << " timestamp=" << msg.timestamp
        << " value=" << std::boolalpha << msg.value
        << std::endl;
}

void ReceivePatternIoMessage(io::IPatternInPort* ioPort, const io::PatternIoMessage& msg)
{
    std::string data{msg.value.begin(), msg.value.end()};

    std::cout << ">> PATTERN I/O Message on port " << ioPort->Config().name
        << " timestamp=" << msg.timestamp
        << " value=\"" << data << "\""
        << std::endl;
}

void ReceivePwmIoMessage(io::IPwmInPort* ioPort, const io::PwmIoMessage& msg)
{
    std::cout << ">> PWM     I/O Message on port " << ioPort->Config().name
        << " timestamp=" << msg.timestamp
        << " frequency=" << msg.value.frequency
        << " dutyCycle=" << msg.value.dutyCycle
        << std::endl;
}

void SendMessage(io::IAnalogOutPort* port, std::chrono::nanoseconds now)
{
    using namespace io;

    static double value = 0.123;
    value *= 1.1;

    port->Write(value, now);

    std::cout << "<< Analog I/O Messages sent" << std::endl;
}

void SendMessage(io::IDigitalOutPort* port, std::chrono::nanoseconds now)
{
    using namespace io;

    static bool value = true;
    value = !value;

    port->Write(value, now);

    std::cout << "<< Digital I/O Messages sent" << std::endl;
}

void SendMessage(io::IPwmOutPort* port, std::chrono::nanoseconds now)
{
    using namespace io;

    static PwmValue value{0.0, 0.1};
    value.frequency += 0.1;
    value.dutyCycle *= 1.05;

    port->Write(value, now);

    std::cout << "<< PWM I/O Messages sent" << std::endl;
}

void SendMessage(io::IPatternOutPort* port, std::chrono::nanoseconds now)
{
    using namespace io;

    static std::vector<uint8_t> value{'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '!', ' '};

    auto head = *value.begin();
    value.erase(value.begin());
    value.push_back(head);

    port->Write(value, now);

    std::cout << "<< Pattern I/O Messages sent" << std::endl;
}


int main(int argc, char** argv)
{
    ib::cfg::Config ibConfig;
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <IbConfig.json> <ParticipantName> [domainId]" << std::endl;
        return -1;
    }
    try
    {
        ibConfig = ib::cfg::Config::FromJsonFile(argv[1]);
    }
    catch (const ib::cfg::Misconfiguration& error)
    {
        std::cerr << "Invalid configuration: " << (&error)->what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -2;
    }
    std::string participantName = argv[2];

    uint32_t domainId = 42;
    if (argc >= 4)
    {
        domainId = static_cast<uint32_t>(std::stoul(argv[3]));
    }

    std::cout << "Creating ComAdapter for Participant=" << participantName << " in Domain " << domainId << std::endl;
    auto comAdapter = ib::CreateFastRtpsComAdapter(ibConfig, participantName, domainId);


    // Set an Init Handler
    auto&& participantController = comAdapter->GetParticipantController();
    participantController->SetInitHandler([&participantName](auto initCmd) {

        std::cout << "Initializing " << participantName << std::endl;

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
    if (participantName == "IoWriter")
    {
        auto* dio = comAdapter->CreateDigitalOut("DIO");
        auto* aio = comAdapter->CreateAnalogOut("AIO");
        auto* pwm = comAdapter->CreatePwmOut("PWM");
        auto* pattern = comAdapter->CreatePatternOut("PATTERN");

        participantController->SetSimulationTask(
            [dio, aio, pwm, pattern](std::chrono::nanoseconds now)
            {
                auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                SendMessage(dio, now);
                SendMessage(aio, now);
                SendMessage(pwm, now);
                SendMessage(pattern, now);
                std::this_thread::sleep_for(1s);
            }
        );
    }
    else
    {
        auto* dio = comAdapter->CreateDigitalIn("DIO");
        auto* aio = comAdapter->CreateAnalogIn("AIO");
        auto* pwm = comAdapter->CreatePwmIn("PWM");
        auto* pattern = comAdapter->CreatePatternIn("PATTERN");

        dio->RegisterHandler(&ReceiveDigitalIoMessage);
        aio->RegisterHandler(&ReceiveAnalogIoMessage);
        pwm->RegisterHandler(&ReceivePwmIoMessage);
        pattern->RegisterHandler(&ReceivePatternIoMessage);

        participantController->SetSimulationTask(
            [](std::chrono::nanoseconds now)
            {
                std::cout << "now=" << std::chrono::duration_cast<std::chrono::milliseconds>(now).count() << "ms" << std::endl;
                std::this_thread::sleep_for(1s);
            }
        );
    }

    //auto finalStateFuture = participantController->RunAsync();
    //auto finalState = finalStateFuture.get();

    auto finalState = participantController->Run();

    std::cout << "Simulation stopped. Final State: " << finalState  << std::endl;
    std::cout << "Press enter to stop the process..." << std::endl;
    std::cin.ignore();

    return 0;
}
